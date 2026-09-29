/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismExp3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Expression/CubismExp3Json.h"
#include "Expression/CubismExp3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

UCubismExp3JsonFactory::UCubismExp3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismExp3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Expression JSON file"));
}


FText UCubismExp3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismExp3JsonFactoryDescription", "Expression JSON exported from Live2D Cubism Editor");
}

bool UCubismExp3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("exp3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismExp3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Exp3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismExp3Json> Result = nullptr;

	FCubismExp3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismExp3Json>(InParent, InName, Flags);
		
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

bool UCubismExp3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismExp3Json* Exp = Cast<UCubismExp3Json>(Obj);
	if (Exp && Exp->AssetImportData)
	{
		Exp->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismExp3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismExp3Json* Exp = Cast<UCubismExp3Json>(Obj);
	if (Exp && ensure(NewReimportPaths.Num() == 1))
	{
		if (!Exp->AssetImportData)
		{
			Exp->AssetImportData = NewObject<UAssetImportData>(Exp, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Exp);

		Exp->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Exp->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismExp3JsonFactory::Reimport(UObject* Obj)
{
	UCubismExp3Json* Exp = Cast<UCubismExp3Json>(Obj);
	if (!Exp)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Exp->AssetImportData
		? Exp->AssetImportData->GetFirstFilename()
		: Exp->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Exp);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Exp))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Exp, TEXT(".exp3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("Exp3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(Exp->GetClass(), Exp->GetOuter(), *Exp->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismExp3Json* ReimportedExp = Cast<UCubismExp3Json>(ImportedObject);
		if (!ReimportedExp)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismExp3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedExp->AssetImportData)
		{
			ReimportedExp->AssetImportData = NewObject<UAssetImportData>(ReimportedExp, TEXT("AssetImportData"));
		}

		ReimportedExp->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedExp);

		ReimportedExp->AssetImportData->Update(Filename);
		ReimportedExp->CubismStoredSourcePath = StoredImportPath;
		ReimportedExp->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedExp);

#if WITH_EDITOR
		ReimportedExp->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedExp, TEXT("Exp3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedExp);

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
