/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismModel3JsonFactory.h"

#include "Model/CubismModel3Json.h"
#include "Model/CubismModel3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"

UCubismModel3JsonFactory::UCubismModel3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismModel3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Model JSON file"));
}


FText UCubismModel3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismModel3JsonFactoryDescription", "Model JSON exported from Live2D Cubism Editor");
}

bool UCubismModel3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("model3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismModel3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext * Warn
)
{
	TObjectPtr<UCubismModel3Json> Result = nullptr;

	FCubismModel3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismModel3Json>(InParent, InName, Flags);

		Importer.ApplyParams(Flags, Result);

		// Update asset import data
		if (Result->AssetImportData)
		{
			Result->AssetImportData->Update(CurrentFilename);
		}
		else
		{
			Result->AssetImportData = NewObject<UAssetImportData>(Result, TEXT("AssetImportData"));
			Result->AssetImportData->Update(CurrentFilename);
		}
	}

	return Result;
}

bool UCubismModel3JsonFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCubismModel3Json* Model = Cast<UCubismModel3Json>(Obj);
	if (Model && Model->AssetImportData)
	{
		Model->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UCubismModel3JsonFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCubismModel3Json* Model = Cast<UCubismModel3Json>(Obj);
	if (Model && ensure(NewReimportPaths.Num() == 1))
	{
		Model->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCubismModel3JsonFactory::Reimport(UObject* Obj)
{
	UCubismModel3Json* Model = Cast<UCubismModel3Json>(Obj);
	if (!Model)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = Model->AssetImportData->GetFirstFilename();
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

	if (ImportObject(Model->GetClass(), Model->GetOuter(), *Model->GetName(), RF_Public, Filename, nullptr, OutCanceled))
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		Model->AssetImportData->Update(Filename);

		Model->MarkPackageDirty();

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
