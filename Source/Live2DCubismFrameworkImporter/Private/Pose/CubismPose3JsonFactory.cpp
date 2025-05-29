/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismPose3JsonFactory.h"

#include "Pose/CubismPose3Json.h"
#include "Pose/CubismPose3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"

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
	TObjectPtr<UCubismPose3Json> Result = nullptr;

	FCubismPose3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismPose3Json>(InParent, InName, Flags);
		
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
		Pose->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCubismPose3JsonFactory::Reimport(UObject* Obj)
{
	UCubismPose3Json* Pose = Cast<UCubismPose3Json>(Obj);
	if (!Pose)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = Pose->AssetImportData->GetFirstFilename();
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

	if (ImportObject(Pose->GetClass(), Pose->GetOuter(), *Pose->GetName(), RF_Public, Filename, nullptr, OutCanceled))
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		Pose->AssetImportData->Update(Filename);

		Pose->MarkPackageDirty();

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
