/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismMotion3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Motion/CubismMotion3Json.h"
#include "Motion/CubismMotion3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismMotion3JsonFactory::UCubismMotion3JsonFactory()
{
	bCreateNew = false;
	SupportedClass = UCubismMotion3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Motion JSON file"));
}

FText UCubismMotion3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismMotion3JsonFactoryDescription", "Motion JSON exported from Live2D Cubism Editor");
}

bool UCubismMotion3JsonFactory::FactoryCanImport(const FString &Filename)
{
	if (!Filename.EndsWith("motion3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismMotion3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Motion3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismMotion3Json> Result = nullptr;

	FCubismMotion3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismMotion3Json>(InParent, InName, Flags);
		
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

bool UCubismMotion3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismMotion3Json* Motion = Cast<UCubismMotion3Json>(Obj);
	if (Motion && Motion->AssetImportData)
	{
		Motion->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismMotion3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismMotion3Json* Motion = Cast<UCubismMotion3Json>(Obj);
	if (Motion && ensure(NewReimportPaths.Num() == 1))
	{
		if (!Motion->AssetImportData)
		{
			Motion->AssetImportData = NewObject<UAssetImportData>(Motion, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Motion);

		Motion->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Motion->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismMotion3JsonFactory::Reimport(UObject* Obj)
{
	UCubismMotion3Json* Motion = Cast<UCubismMotion3Json>(Obj);
	if (!Motion)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Motion->AssetImportData
		? Motion->AssetImportData->GetFirstFilename()
		: Motion->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Motion);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Motion))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Motion, TEXT(".motion3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("Motion3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(Motion->GetClass(), Motion->GetOuter(), *Motion->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismMotion3Json* ReimportedMotion = Cast<UCubismMotion3Json>(ImportedObject);
		if (!ReimportedMotion)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismMotion3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedMotion->AssetImportData)
		{
			ReimportedMotion->AssetImportData = NewObject<UAssetImportData>(ReimportedMotion, TEXT("AssetImportData"));
		}

		ReimportedMotion->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedMotion);

		ReimportedMotion->AssetImportData->Update(Filename);
		ReimportedMotion->CubismStoredSourcePath = StoredImportPath;
		ReimportedMotion->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedMotion);

#if WITH_EDITOR
		ReimportedMotion->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedMotion, TEXT("Motion3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedMotion);

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

