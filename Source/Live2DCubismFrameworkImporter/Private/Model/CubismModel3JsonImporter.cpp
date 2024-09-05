/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismModel3JsonImporter.h"
#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismModel3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
{
	const TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(FileContent);
	TSharedPtr<FJsonObject> JsonObject;

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		return JsonObject;
	}
	else
	{
		if (!bSilent)
		{
			UE_LOG(LogCubism, Error, TEXT("Failed to deserialize JSON object.  Error: '%s'"), *JsonReader->GetErrorMessage());
		}
		return nullptr;
	}
}

bool FCubismModel3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	ASSERT(JsonObject->TryGetNumberField(TEXT("Version"), Version), "Failed to get Version field.");

	const TSharedPtr<FJsonObject>* FileReferences;
	ASSERT(JsonObject->TryGetObjectField(TEXT("FileReferences"), FileReferences), "Failed to get FileReferences field.");	
	{
		(*FileReferences)->TryGetStringField(TEXT("Moc"), MocPath);

		(*FileReferences)->TryGetStringArrayField(TEXT("Textures"), TexturePaths);

		(*FileReferences)->TryGetStringField(TEXT("Physics"), PhysicsPath);

		(*FileReferences)->TryGetStringField(TEXT("Pose"), PosePath);

		(*FileReferences)->TryGetStringField(TEXT("DisplayInfo"), DisplayInfoPath);

		const TArray< TSharedPtr<FJsonValue> >*  ExpressionsArrayObject;
		if ((*FileReferences)->TryGetArrayField(TEXT("Expressions"), ExpressionsArrayObject))
		{
			for (const TSharedPtr<FJsonValue>& ExpressionValue: *ExpressionsArrayObject)
			{
				FExpressionEntry Entry;

				const TSharedPtr<FJsonObject>& ExpressionObject = ExpressionValue->AsObject();
				ASSERT(ExpressionObject->TryGetStringField(TEXT("Name"), Entry.Name), "Failed to get Name field.");
				ASSERT(ExpressionObject->TryGetStringField(TEXT("File"), Entry.Path), "Failed to get File field.");

				Expressions.Add(Entry);
			}
		}

		const TSharedPtr<FJsonObject>* MotionsObject;
		if ((*FileReferences)->TryGetObjectField(TEXT("Motions"), MotionsObject))
		{
			for (auto It = (*MotionsObject)->Values.CreateIterator(); It; ++It)
			{
				FMotionGroupEntry Entry;
				
				for (const TSharedPtr<FJsonValue>& MotionGroupValue : It.Value()->AsArray())
				{
					const TSharedPtr<FJsonObject>& MotionGroupObject = MotionGroupValue->AsObject();

					FString MotionPath;
					ASSERT(MotionGroupObject->TryGetStringField(TEXT("File"), MotionPath), "Failed to get File field.");

					Entry.Paths.Add(MotionPath);
				}

				Entry.Name = It.Key();

				Motions.Add(Entry);
			}
		}

		(*FileReferences)->TryGetStringField(TEXT("UserData"), UserDataPath);
	}

	const TArray< TSharedPtr<FJsonValue> >* GroupsArrayObject;
	if (JsonObject->TryGetArrayField(TEXT("Groups"), GroupsArrayObject))
	{
		for (const TSharedPtr<FJsonValue>& GroupValue: *GroupsArrayObject)
		{
			const TSharedPtr<FJsonObject>& GroupObject = GroupValue->AsObject();
			FString Name;
			ASSERT(GroupObject->TryGetStringField(TEXT("Name"), Name), "Failed to get Name field.");
			if ((Name == TEXT("EyeBlink")))
			{
				ASSERT(GroupObject->TryGetStringArrayField(TEXT("Ids"), EyeBlinks), "Failed to get Ids field.");
			}
			else if ((Name == TEXT("LipSync")))
			{
				ASSERT(GroupObject->TryGetStringArrayField(TEXT("Ids"), LipSyncs), "Failed to get Ids field.");
			}
		}
	}

	const TArray< TSharedPtr<FJsonValue> >* HitAreasArrayObject;
	if (JsonObject->TryGetArrayField(TEXT("HitAreas"), HitAreasArrayObject))
	{
		for (const TSharedPtr<FJsonValue>& HitAreaValue: *HitAreasArrayObject)
		{
			FHitAreaEntry Entry;

			const TSharedPtr<FJsonObject>& HitAreaObject = HitAreaValue->AsObject();
			ASSERT(HitAreaObject->TryGetStringField(TEXT("Id"), Entry.Id), "Failed to get Id field.");
			ASSERT(HitAreaObject->TryGetStringField(TEXT("Name"), Entry.Name), "Failed to get Name field.");

			HitAreas.Add(Entry);
		}
	}

	return true;
}

bool FCubismModel3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismModel3Json>& ModelSetting)
{
	ModelSetting->Version = Version;

	ModelSetting->MocPath = MocPath;

	ModelSetting->TexturePaths = TexturePaths;
	
	ModelSetting->PhysicsPath = PhysicsPath;

	ModelSetting->PosePath = PosePath;

	ModelSetting->DisplayInfoPath = DisplayInfoPath;

	ModelSetting->Expressions = Expressions;

	ModelSetting->Motions = Motions;

	ModelSetting->UserDataPath = UserDataPath;

	ModelSetting->EyeBlinks = EyeBlinks;

	ModelSetting->LipSyncs = LipSyncs;

	ModelSetting->HitAreas = HitAreas;
	
	return true;
}
