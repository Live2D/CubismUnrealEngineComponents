/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismMoc3Factory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Model/CubismMoc3.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"

UCubismMoc3Factory::UCubismMoc3Factory() 
{
	bCreateNew = false;
	SupportedClass = UCubismMoc3::StaticClass();

	bEditorImport = true;
	bText = false;

	Formats.Add(TEXT("moc3;Cubism Moc Binary file"));
}


FText UCubismMoc3Factory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismMoc3FactoryDescription", "Moc exported from Live2D Cubism Editor");
}

bool UCubismMoc3Factory::FactoryCanImport(const FString& Filename)
{
	TArray<uint8> FileContent;
	if (FFileHelper::LoadFileToArray(FileContent, *Filename))
	{
		bool bHasMocConsistency = UCubismMoc3::HasMocConsistency(FileContent.GetData(), FileContent.Num());

		if (!bHasMocConsistency)
		{
			UE_LOG(LogCubism, Error, TEXT("UCubismMoc3Factory::FactoryCanImport: Moc consistency check failed"));
		}

		return bHasMocConsistency;
	}

	return false;
}

UObject* UCubismMoc3Factory::FactoryCreateBinary
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
	FFeedbackContext * Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Moc3")))
	{
		return nullptr;
	}

	TArray<uint8> Bytes;
	for (int32 i = 0; i < BufferEnd - Buffer; ++i)
	{
		Bytes.Add(static_cast<uint8>(Buffer[i]));
	}

	TObjectPtr<UCubismMoc3> Result = NewObject<UCubismMoc3>(InParent, InName, Flags);

	Result->Bytes = Bytes;
	Result->Setup();

	if (!Result->AssetImportData)
	{
		Result->AssetImportData = NewObject<UAssetImportData>(Result, TEXT("AssetImportData"));
	}

	FString ImportFilename = CurrentFilename;
	if (!CubismImporterCopySourceToAssetPackageDirectory(CurrentFilename, Result, ImportFilename))
	{
		return nullptr;
	}

	const FString StoredImportPath = CubismMakeStoredSourcePath(ImportFilename, Result);

	UE_LOG(LogCubism, Warning, TEXT("Moc3 FactoryCreateBinary: Current=%s"), *ImportFilename);
	UE_LOG(LogCubism, Warning, TEXT("Moc3 FactoryCreateBinary: Stored=%s"), *StoredImportPath);

	Result->AssetImportData->Update(ImportFilename);
	Result->CubismStoredSourcePath = StoredImportPath;
	UE_LOG(LogCubism, Warning, TEXT("Moc3 FactoryCreateBinary: CubismStoredSourcePath=%s"), *Result->CubismStoredSourcePath);

#if WITH_EDITOR
	Result->PostEditChange();
#endif

	return Result;
}

bool UCubismMoc3Factory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismMoc3* Moc = Cast<UCubismMoc3>(Obj);
	if (Moc && Moc->AssetImportData)
	{
		Moc->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismMoc3Factory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismMoc3* Moc = Cast<UCubismMoc3>(Obj);
	if (Moc && ensure(NewReimportPaths.Num() == 1))
	{
		if (!Moc->AssetImportData)
		{
			Moc->AssetImportData = NewObject<UAssetImportData>(Moc, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Moc);

		UE_LOG(LogCubism, Warning, TEXT("Moc3 SetReimportPaths: Input=%s"), *NewReimportPaths[0]);
		UE_LOG(LogCubism, Warning, TEXT("Moc3 SetReimportPaths: Stored=%s"), *StoredImportPath);

		Moc->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Moc->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismMoc3Factory::Reimport(UObject* Obj)
{
	UCubismMoc3* Moc = Cast<UCubismMoc3>(Obj);
	if (!Moc)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Moc->AssetImportData->GetFirstFilename();
	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Moc);

	UE_LOG(LogCubism, Warning, TEXT("Moc3 Reimport: Stored=%s"), *StoredFilename);
	UE_LOG(LogCubism, Warning, TEXT("Moc3 Reimport: Resolved=%s"), *Filename);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Moc))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Moc, TEXT(".moc3"), false);

		UE_LOG(LogCubism, Warning, TEXT("Moc3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

		if (!FallbackFilename.IsEmpty())
		{
			Filename = FallbackFilename;
		}
	}

	if (!Filename.Len())
	{
		return EReimportResult::Failed;
	}

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		UE_LOG(LogCubism, Warning, TEXT("Cannot reimport: source file '%s' cannot be found."), *Filename);
		return EReimportResult::Failed;
	}

	bool OutCanceled = false;

	UObject* ImportedObject = ImportObject(
		Moc->GetClass(),
		Moc->GetOuter(),
		*Moc->GetName(),
		RF_Public | RF_Standalone,
		Filename,
		nullptr,
		OutCanceled
	);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismMoc3* ReimportedMoc = Cast<UCubismMoc3>(ImportedObject);
		if (!ReimportedMoc)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismMoc3"));
			return EReimportResult::Failed;
		}

		if (!ReimportedMoc->AssetImportData)
		{
			ReimportedMoc->AssetImportData = NewObject<UAssetImportData>(ReimportedMoc, TEXT("AssetImportData"));
		}

		ReimportedMoc->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedMoc);
		UE_LOG(LogCubism, Warning, TEXT("Moc3 Reimport: Updated stored path=%s"), *StoredImportPath);

		ReimportedMoc->AssetImportData->Update(Filename);
		ReimportedMoc->CubismStoredSourcePath = StoredImportPath;
		UE_LOG(LogCubism, Warning, TEXT("Moc3 Reimport: CubismStoredSourcePath=%s"), *ReimportedMoc->CubismStoredSourcePath);

		ReimportedMoc->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedMoc);

#if WITH_EDITOR
		ReimportedMoc->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedMoc, TEXT("Moc3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedMoc);

		return EReimportResult::Succeeded;
	}
	else
	{
		if (OutCanceled)
		{
			UE_LOG(LogCubism, Warning, TEXT("Reimport was canceled"));
			return EReimportResult::Cancelled;
		}
		else
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed"));
			return EReimportResult::Failed;
		}
	}
}

