/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismUserData3JsonFactory.h"

#include "UserData/CubismUserData3Json.h"
#include "UserData/CubismUserData3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"

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
	TObjectPtr<UCubismUserData3Json> Result = nullptr;

	FCubismUserData3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismUserData3Json>(InParent, InName, Flags);
		
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
		UserData->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCubismUserData3JsonFactory::Reimport(UObject* Obj)
{
	UCubismUserData3Json* UserData = Cast<UCubismUserData3Json>(Obj);
	if (!UserData)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = UserData->AssetImportData->GetFirstFilename();
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

	if (ImportObject(UserData->GetClass(), UserData->GetOuter(), *UserData->GetName(), RF_Public, Filename, nullptr, OutCanceled))
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UserData->AssetImportData->Update(Filename);

		UserData->MarkPackageDirty();

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
