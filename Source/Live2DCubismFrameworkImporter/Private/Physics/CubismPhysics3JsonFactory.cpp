/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismPhysics3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Physics/CubismPhysics3Json.h"
#include "Physics/CubismPhysics3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismPhysics3JsonFactory::UCubismPhysics3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismPhysics3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Physics JSON file"));
}


FText UCubismPhysics3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismPhysics3JsonFactoryDescription", "Physics JSON exported from Live2D Cubism Editor");
}

bool UCubismPhysics3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("physics3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismPhysics3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Physics3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismPhysics3Json> Result = nullptr;

	FCubismPhysics3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismPhysics3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);

		FString ImportFilename = CurrentFilename;
		if (!CubismImporterCopySourceToAssetPackageDirectory(CurrentFilename, Result, ImportFilename))
		{
			return nullptr;
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(ImportFilename, Result);

		// Update asset import data
		if (Result->AssetImportData)
		{
			Result->AssetImportData->Update(ImportFilename);
		}
		else
		{
			Result->AssetImportData = NewObject<UAssetImportData>(Result, TEXT("AssetImportData"));
			Result->AssetImportData->Update(ImportFilename);
		}

		Result->CubismStoredSourcePath = StoredImportPath;
	}

	return Result;
}

bool UCubismPhysics3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismPhysics3Json* Physics = Cast<UCubismPhysics3Json>(Obj);
	if (Physics && Physics->AssetImportData)
	{
		Physics->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismPhysics3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismPhysics3Json* Physics = Cast<UCubismPhysics3Json>(Obj);
	if (Physics && ensure(NewReimportPaths.Num() == 1))
	{
		if (!Physics->AssetImportData)
		{
			Physics->AssetImportData = NewObject<UAssetImportData>(Physics, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Physics);

		Physics->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Physics->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismPhysics3JsonFactory::Reimport(UObject* Obj)
{
	UCubismPhysics3Json* Physics = Cast<UCubismPhysics3Json>(Obj);
	if (!Physics)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Physics->AssetImportData
		? Physics->AssetImportData->GetFirstFilename()
		: Physics->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Physics);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Physics))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Physics, TEXT(".physics3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("Physics3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(Physics->GetClass(), Physics->GetOuter(), *Physics->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismPhysics3Json* ReimportedPhysics = Cast<UCubismPhysics3Json>(ImportedObject);
		if (!ReimportedPhysics)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismPhysics3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedPhysics->AssetImportData)
		{
			ReimportedPhysics->AssetImportData = NewObject<UAssetImportData>(ReimportedPhysics, TEXT("AssetImportData"));
		}

		ReimportedPhysics->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedPhysics);

		ReimportedPhysics->AssetImportData->Update(Filename);
		ReimportedPhysics->CubismStoredSourcePath = StoredImportPath;
		ReimportedPhysics->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedPhysics);

#if WITH_EDITOR
		ReimportedPhysics->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedPhysics, TEXT("Physics3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedPhysics);

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
