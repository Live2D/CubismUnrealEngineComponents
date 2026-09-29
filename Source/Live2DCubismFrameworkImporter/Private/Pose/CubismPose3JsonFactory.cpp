/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismPose3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Pose/CubismPose3Json.h"
#include "Pose/CubismPose3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismPose3JsonFactory::UCubismPose3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismPose3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Pose JSON file"));
}

FText UCubismPose3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismPose3JsonFactoryDescription", "Pose JSON exported from Live2D Cubism Editor");
}

bool UCubismPose3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("pose3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismPose3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Pose3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismPose3Json> Result = nullptr;

	FCubismPose3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismPose3Json>(InParent, InName, Flags);
		
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

bool UCubismPose3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismPose3Json* Pose = Cast<UCubismPose3Json>(Obj);
	if (Pose && Pose->AssetImportData)
	{
		Pose->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismPose3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismPose3Json* Pose = Cast<UCubismPose3Json>(Obj);
	if (Pose && ensure(NewReimportPaths.Num() == 1))
	{
		if (!Pose->AssetImportData)
		{
			Pose->AssetImportData = NewObject<UAssetImportData>(Pose, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Pose);

		Pose->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Pose->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismPose3JsonFactory::Reimport(UObject* Obj)
{
	UCubismPose3Json* Pose = Cast<UCubismPose3Json>(Obj);
	if (!Pose)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Pose->AssetImportData
		? Pose->AssetImportData->GetFirstFilename()
		: Pose->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Pose);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Pose))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Pose, TEXT(".pose3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("Pose3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(Pose->GetClass(), Pose->GetOuter(), *Pose->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismPose3Json* ReimportedPose = Cast<UCubismPose3Json>(ImportedObject);
		if (!ReimportedPose)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismPose3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedPose->AssetImportData)
		{
			ReimportedPose->AssetImportData = NewObject<UAssetImportData>(ReimportedPose, TEXT("AssetImportData"));
		}

		ReimportedPose->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedPose);

		ReimportedPose->AssetImportData->Update(Filename);
		ReimportedPose->CubismStoredSourcePath = StoredImportPath;
		ReimportedPose->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedPose);

#if WITH_EDITOR
		ReimportedPose->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedPose, TEXT("Pose3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedPose);

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
