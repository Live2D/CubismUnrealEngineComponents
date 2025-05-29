/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismDisplayInfo3JsonFactory.h"

#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "DisplayInfo/CubismDisplayInfo3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"

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
	TObjectPtr<UCubismDisplayInfo3Json> Result = nullptr;

	FCubismDisplayInfo3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismDisplayInfo3Json>(InParent, InName, Flags);
		
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
		DisplayInfo->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCubismDisplayInfo3JsonFactory::Reimport(UObject* Obj)
{
	UCubismDisplayInfo3Json* DisplayInfo = Cast<UCubismDisplayInfo3Json>(Obj);
	if (!DisplayInfo)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = DisplayInfo->AssetImportData->GetFirstFilename();
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

	if (ImportObject(DisplayInfo->GetClass(), DisplayInfo->GetOuter(), *DisplayInfo->GetName(), RF_Public, Filename, nullptr, OutCanceled))
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		DisplayInfo->AssetImportData->Update(Filename);

		DisplayInfo->MarkPackageDirty();

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
