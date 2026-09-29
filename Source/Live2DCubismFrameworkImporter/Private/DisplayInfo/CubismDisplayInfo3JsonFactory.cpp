/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismDisplayInfo3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "DisplayInfo/CubismDisplayInfo3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismDisplayInfo3JsonFactory::UCubismDisplayInfo3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismDisplayInfo3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Display Info JSON file"));
}


FText UCubismDisplayInfo3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismDisplayInfo3JsonFactoryDescription", "DisplayInfo JSON exported from Live2D Cubism Editor");
}

bool UCubismDisplayInfo3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("cdi3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismDisplayInfo3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("DisplayInfo3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismDisplayInfo3Json> Result = nullptr;

	FCubismDisplayInfo3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismDisplayInfo3Json>(InParent, InName, Flags);
		
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

bool UCubismDisplayInfo3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismDisplayInfo3Json* DisplayInfo = Cast<UCubismDisplayInfo3Json>(Obj);
	if (DisplayInfo && DisplayInfo->AssetImportData)
	{
		DisplayInfo->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismDisplayInfo3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismDisplayInfo3Json* DisplayInfo = Cast<UCubismDisplayInfo3Json>(Obj);
	if (DisplayInfo && ensure(NewReimportPaths.Num() == 1))
	{
		if (!DisplayInfo->AssetImportData)
		{
			DisplayInfo->AssetImportData = NewObject<UAssetImportData>(DisplayInfo, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], DisplayInfo);

		DisplayInfo->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		DisplayInfo->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismDisplayInfo3JsonFactory::Reimport(UObject* Obj)
{
	UCubismDisplayInfo3Json* DisplayInfo = Cast<UCubismDisplayInfo3Json>(Obj);
	if (!DisplayInfo)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = DisplayInfo->AssetImportData
		? DisplayInfo->AssetImportData->GetFirstFilename()
		: DisplayInfo->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, DisplayInfo);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(DisplayInfo))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(DisplayInfo, TEXT(".cdi3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("DisplayInfo Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(DisplayInfo->GetClass(), DisplayInfo->GetOuter(), *DisplayInfo->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismDisplayInfo3Json* ReimportedDisplayInfo = Cast<UCubismDisplayInfo3Json>(ImportedObject);
		if (!ReimportedDisplayInfo)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismDisplayInfo3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedDisplayInfo->AssetImportData)
		{
			ReimportedDisplayInfo->AssetImportData = NewObject<UAssetImportData>(ReimportedDisplayInfo, TEXT("AssetImportData"));
		}

		ReimportedDisplayInfo->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedDisplayInfo);

		ReimportedDisplayInfo->AssetImportData->Update(Filename);
		ReimportedDisplayInfo->CubismStoredSourcePath = StoredImportPath;
		ReimportedDisplayInfo->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedDisplayInfo);

#if WITH_EDITOR
		ReimportedDisplayInfo->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedDisplayInfo, TEXT("DisplayInfo"));
		CubismImporterUpdateAssetRegistryTags(ReimportedDisplayInfo);

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
