/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismModel3JsonFactory.h"

#include "CubismImporterUtils.h"
#include "CubismSourcePathUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "EditorFramework/AssetImportData.h"
#include "EditorReimportHandler.h"
#include "Engine/Texture.h"
#include "Expression/CubismExp3Json.h"
#include "Json.h"
#include "Model/CubismModel3Json.h"
#include "Model/CubismModel3JsonImporter.h"
#include "Model/CubismMoc3.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Motion/CubismMotion3Json.h"
#include "Physics/CubismPhysics3Json.h"
#include "Pose/CubismPose3Json.h"
#include "Sound/SoundWave.h"
#include "UserData/CubismUserData3Json.h"
#include "CubismLog.h"
#include "UObject/SavePackage.h"

static bool CubismModel3AddReferencedSourcePath(
	const FString& ModelDirectory,
	const FString& RelativeSourcePath,
	TSet<FString>& OutSourcePaths
)
{
	if (RelativeSourcePath.IsEmpty())
	{
		return false;
	}

	FString SourcePath = FPaths::Combine(ModelDirectory, RelativeSourcePath);
	FPaths::NormalizeFilename(SourcePath);
	SourcePath = FPaths::ConvertRelativePathToFull(SourcePath);
	FPaths::NormalizeFilename(SourcePath);

	if (IFileManager::Get().FileSize(*SourcePath) == INDEX_NONE)
	{
		const FString SourceFilename = FPaths::GetCleanFilename(RelativeSourcePath);
		TArray<FString> Matches;
		IFileManager::Get().FindFilesRecursive(Matches, *ModelDirectory, *SourceFilename, true, false, false);

		if (Matches.Num() > 0)
		{
			SourcePath = Matches[0];
			FPaths::NormalizeFilename(SourcePath);
			SourcePath = FPaths::ConvertRelativePathToFull(SourcePath);
			FPaths::NormalizeFilename(SourcePath);
		}
	}

	if (IFileManager::Get().FileSize(*SourcePath) == INDEX_NONE)
	{
		UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: source file not found: %s"), *RelativeSourcePath);
		return false;
	}

	OutSourcePaths.Add(SourcePath);
	return true;
}

static FString CubismModel3MakeExpectedAssetBaseName(const FString& SourceFilename)
{
	const FString CleanFilename = FPaths::GetCleanFilename(SourceFilename);

	struct FKnownSuffix
	{
		const TCHAR* SourceSuffix;
		const TCHAR* AssetSuffix;
	};

	static const FKnownSuffix KnownSuffixes[] =
	{
		{ TEXT(".cdi3.json"), TEXT("_cdi3") },
		{ TEXT(".model3.json"), TEXT("_model3") },
		{ TEXT(".physics3.json"), TEXT("_physics3") },
		{ TEXT(".pose3.json"), TEXT("_pose3") },
		{ TEXT(".userdata3.json"), TEXT("_userdata3") },
		{ TEXT(".exp3.json"), TEXT("_exp3") },
		{ TEXT(".motion3.json"), TEXT("_motion3") },
		{ TEXT(".moc3"), TEXT("") },
	};

	for (const FKnownSuffix& KnownSuffix : KnownSuffixes)
	{
		const FString SourceSuffix(KnownSuffix.SourceSuffix);

		if (CleanFilename.EndsWith(SourceSuffix, ESearchCase::IgnoreCase))
		{
			return CleanFilename.LeftChop(SourceSuffix.Len()) + KnownSuffix.AssetSuffix;
		}
	}

	return FPaths::GetBaseFilename(CleanFilename);
}

static UObject* CubismModel3LoadAssetForSourcePath(const FString& SourcePath)
{
	const FString AssetBaseName = CubismModel3MakeExpectedAssetBaseName(SourcePath);
	FString AssetPackageFilename = FPaths::Combine(FPaths::GetPath(SourcePath), AssetBaseName + FPackageName::GetAssetPackageExtension());
	FPaths::NormalizeFilename(AssetPackageFilename);
	AssetPackageFilename = FPaths::ConvertRelativePathToFull(AssetPackageFilename);
	FPaths::NormalizeFilename(AssetPackageFilename);

	if (IFileManager::Get().FileSize(*AssetPackageFilename) == INDEX_NONE)
	{
		UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: asset package not found for source %s"), *SourcePath);
		return nullptr;
	}

	FString LongPackageName;
	if (!FPackageName::TryConvertFilenameToLongPackageName(AssetPackageFilename, LongPackageName))
	{
		UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: failed to convert asset package path: %s"), *AssetPackageFilename);
		return nullptr;
	}

	const FString ObjectPath = LongPackageName + TEXT(".") + AssetBaseName;
	return StaticLoadObject(UObject::StaticClass(), nullptr, *ObjectPath);
}

static UAssetImportData* CubismModel3GetAssetImportData(UObject* AssetObject)
{
	if (UCubismMoc3* Moc = Cast<UCubismMoc3>(AssetObject))
	{
		return Moc->AssetImportData;
	}
	if (UCubismModel3Json* Model = Cast<UCubismModel3Json>(AssetObject))
	{
		return Model->AssetImportData;
	}
	if (UCubismDisplayInfo3Json* DisplayInfo = Cast<UCubismDisplayInfo3Json>(AssetObject))
	{
		return DisplayInfo->AssetImportData;
	}
	if (UCubismPhysics3Json* Physics = Cast<UCubismPhysics3Json>(AssetObject))
	{
		return Physics->AssetImportData;
	}
	if (UCubismPose3Json* Pose = Cast<UCubismPose3Json>(AssetObject))
	{
		return Pose->AssetImportData;
	}
	if (UCubismExp3Json* Exp = Cast<UCubismExp3Json>(AssetObject))
	{
		return Exp->AssetImportData;
	}
	if (UCubismMotion3Json* Motion = Cast<UCubismMotion3Json>(AssetObject))
	{
		return Motion->AssetImportData;
	}
	if (UCubismUserData3Json* UserData = Cast<UCubismUserData3Json>(AssetObject))
	{
		return UserData->AssetImportData;
	}
	if (UTexture* Texture = Cast<UTexture>(AssetObject))
	{
		return Texture->AssetImportData;
	}
	if (USoundWave* SoundWave = Cast<USoundWave>(AssetObject))
	{
		return SoundWave->AssetImportData;
	}

	return nullptr;
}

static void CubismModel3CollectReferencedSourcePaths(const FString& ModelFilename, TSet<FString>& OutSourcePaths)
{
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *ModelFilename))
	{
		UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: failed to read model3 file: %s"), *ModelFilename);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: failed to parse model3 file: %s"), *ModelFilename);
		return;
	}

	const TSharedPtr<FJsonObject>* FileReferences;
	if (!JsonObject->TryGetObjectField(TEXT("FileReferences"), FileReferences))
	{
		return;
	}

	const FString ModelDirectory = FPaths::GetPath(ModelFilename);

	auto AddStringField = [&](const TCHAR* FieldName)
	{
		FString ReferencePath;
		if ((*FileReferences)->TryGetStringField(FieldName, ReferencePath))
		{
			CubismModel3AddReferencedSourcePath(ModelDirectory, ReferencePath, OutSourcePaths);
		}
	};

	AddStringField(TEXT("Moc"));
	AddStringField(TEXT("Physics"));
	AddStringField(TEXT("Pose"));
	AddStringField(TEXT("DisplayInfo"));
	AddStringField(TEXT("UserData"));

	TArray<FString> TexturePaths;
	if ((*FileReferences)->TryGetStringArrayField(TEXT("Textures"), TexturePaths))
	{
		for (const FString& TexturePath : TexturePaths)
		{
			CubismModel3AddReferencedSourcePath(ModelDirectory, TexturePath, OutSourcePaths);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* ExpressionsArray;
	if ((*FileReferences)->TryGetArrayField(TEXT("Expressions"), ExpressionsArray))
	{
		for (const TSharedPtr<FJsonValue>& ExpressionValue : *ExpressionsArray)
		{
			const TSharedPtr<FJsonObject> ExpressionObject = ExpressionValue->AsObject();
			if (ExpressionObject.IsValid())
			{
				FString ReferencePath;
				if (ExpressionObject->TryGetStringField(TEXT("File"), ReferencePath))
				{
					CubismModel3AddReferencedSourcePath(ModelDirectory, ReferencePath, OutSourcePaths);
				}
			}
		}
	}

	const TSharedPtr<FJsonObject>* MotionsObject;
	if ((*FileReferences)->TryGetObjectField(TEXT("Motions"), MotionsObject))
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& MotionGroup : (*MotionsObject)->Values)
		{
			for (const TSharedPtr<FJsonValue>& MotionValue : MotionGroup.Value->AsArray())
			{
				const TSharedPtr<FJsonObject> MotionObject = MotionValue->AsObject();
				if (MotionObject.IsValid())
				{
					FString ReferencePath;
					if (MotionObject->TryGetStringField(TEXT("File"), ReferencePath))
					{
						CubismModel3AddReferencedSourcePath(ModelDirectory, ReferencePath, OutSourcePaths);
					}
					if (MotionObject->TryGetStringField(TEXT("Sound"), ReferencePath))
					{
						CubismModel3AddReferencedSourcePath(ModelDirectory, ReferencePath, OutSourcePaths);
					}
				}
			}
		}
	}
}

static void CubismModel3ReimportReferencedAssets(const FString& ModelFilename, UObject* ModelAsset)
{
	TSet<FString> ReferencedSourcePaths;
	CubismModel3CollectReferencedSourcePaths(ModelFilename, ReferencedSourcePaths);

	for (const FString& SourcePath : ReferencedSourcePaths)
	{
		UObject* RelatedAsset = CubismModel3LoadAssetForSourcePath(SourcePath);
		if (!RelatedAsset || RelatedAsset == ModelAsset)
		{
			continue;
		}

		if (UAssetImportData* RelatedImportData = CubismModel3GetAssetImportData(RelatedAsset))
		{
			RelatedAsset->Modify();
			RelatedImportData->Update(SourcePath);
			RelatedAsset->MarkPackageDirty();
			CubismImporterUpdateAssetRegistryTags(RelatedAsset);
		}

		const bool bReimported = FReimportManager::Instance()->Reimport(
			RelatedAsset,
			false,
			true,
			SourcePath,
			nullptr,
			INDEX_NONE,
			true,
			true
		);

		if (bReimported)
		{
			CubismImporterSaveReimportedAssetPackage(RelatedAsset, TEXT("Model3 cascade"));
			CubismImporterUpdateAssetRegistryTags(RelatedAsset);
		}
		else
		{
			UE_LOG(LogCubism, Warning, TEXT("Model3 cascade reimport: failed to reimport %s from %s"),
				*RelatedAsset->GetName(),
				*SourcePath);
		}
	}
}

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
	if (CubismImporterShouldSuppressDuplicateImport(CurrentFilename, InParent, TEXT("Model3")))
	{
		return nullptr;
	}

	TObjectPtr<UCubismModel3Json> Result = nullptr;

	FCubismModel3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismModel3Json>(InParent, InName, Flags);

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
		if (!Model->AssetImportData)
		{
			Model->AssetImportData = NewObject<UAssetImportData>(Model, TEXT("AssetImportData"));
		}

		const FString StoredImportPath = CubismMakeStoredSourcePath(NewReimportPaths[0], Model);

		Model->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		Model->CubismStoredSourcePath = StoredImportPath;
	}
}

EReimportResult::Type UCubismModel3JsonFactory::Reimport(UObject* Obj)
{
	UCubismModel3Json* Model = Cast<UCubismModel3Json>(Obj);
	if (!Model)
	{
		return EReimportResult::Failed;
	}

	const FString StoredFilename = Model->AssetImportData
		? Model->AssetImportData->GetFirstFilename()
		: Model->CubismStoredSourcePath;

	FString Filename = CubismResolveStoredSourcePath(StoredFilename, Model);

	if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE && CubismIsPluginAsset(Model))
	{
		const FString FallbackFilename = CubismImporterFindPluginLocalSourceFallback(Model, TEXT(".model3.json"), true);

		UE_LOG(LogCubism, Warning, TEXT("Model3 Reimport: Plugin fallback candidate=%s"), *FallbackFilename);

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

	UObject* ImportedObject = ImportObject(Model->GetClass(), Model->GetOuter(), *Model->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);

	if (ImportedObject)
	{
		UE_LOG(LogCubism, Log, TEXT("Reimported successfully"));

		UCubismModel3Json* ReimportedModel = Cast<UCubismModel3Json>(ImportedObject);
		if (!ReimportedModel)
		{
			UE_LOG(LogCubism, Error, TEXT("Reimport failed: ImportedObject is not UCubismModel3Json"));
			return EReimportResult::Failed;
		}

		if (!ReimportedModel->AssetImportData)
		{
			ReimportedModel->AssetImportData = NewObject<UAssetImportData>(ReimportedModel, TEXT("AssetImportData"));
		}

		ReimportedModel->Modify();

		const FString StoredImportPath = CubismMakeStoredSourcePath(Filename, ReimportedModel);

		ReimportedModel->AssetImportData->Update(Filename);
		ReimportedModel->CubismStoredSourcePath = StoredImportPath;
		ReimportedModel->MarkPackageDirty();
		CubismImporterUpdateAssetRegistryTags(ReimportedModel);

#if WITH_EDITOR
		ReimportedModel->PostEditChange();
#endif

		CubismImporterSaveReimportedAssetPackage(ReimportedModel, TEXT("Model3"));
		CubismImporterUpdateAssetRegistryTags(ReimportedModel);
		CubismModel3ReimportReferencedAssets(Filename, ReimportedModel);

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
