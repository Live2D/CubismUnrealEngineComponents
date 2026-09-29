/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismUserData3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "UserData/CubismUserData3Json.h"
#include "UserData/CubismUserData3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismUserData3JsonFactory::UCubismUserData3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismUserData3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism User Data JSON file"));
}


FText UCubismUserData3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismUserData3JsonFactoryDescription", "User Data JSON exported from Live2D Cubism Editor");
}

bool UCubismUserData3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("userdata3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismUserData3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("UserData3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismUserData3Json> Result = nullptr;

	FCubismUserData3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismUserData3Json>(InParent, InName, Flags);
		
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

bool UCubismUserData3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismUserData3Json* UserData = Cast<UCubismUserData3Json>(Obj);
	if (UserData && UserData->AssetImportData)
	{
		UserData->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismUserData3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismUserData3Json* UserData = Cast<UCubismUserData3Json>(Obj);
	if (UserData && ensure(NewReimportPaths.Num() == 1))
	{
		if (!UserData->AssetImportData)
		{
			UserData->AssetImportData = NewObject<UAssetImportData>(UserData, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], UserData);

		UserData->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		UserData->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismUserData3JsonFactory::Reimport(UObject* Obj)
{
	UCubismUserData3Json* UserData = Cast<UCubismUserData3Json>(Obj);
	if (!UserData)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = UserData->AssetImportData
		? UserData->AssetImportData->GetFirstFilename()
		: UserData->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, UserData);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(UserData))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(UserData, TEXT(".userdata3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("UserData3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(UserData->GetClass(), UserData->GetOuter(), *UserData->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismUserData3Json* ReimportedUserData = Cast<UCubismUserData3Json>(ImportedObject);
		if (!ReimportedUserData)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismUserData3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedUserData->AssetImportData)
		{
			ReimportedUserData->AssetImportData = NewObject<UAssetImportData>(ReimportedUserData, TEXT("AssetImportData"));
		}

		ReimportedUserData->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedUserData);

		ReimportedUserData->AssetImportData->Update(Filename);
		ReimportedUserData->CubismStoredSourcePath = StoredImportPath;
		ReimportedUserData->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedUserData);

#if WITH_EDITOR
		ReimportedUserData->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedUserData, TEXT("UserData3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedUserData);

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
