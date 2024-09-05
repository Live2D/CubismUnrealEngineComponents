/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Pose/CubismPose3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismPose3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
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

bool FCubismPose3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	JsonObject->TryGetNumberField(TEXT("Version"), Version);

	JsonObject->TryGetNumberField(TEXT("FadeInTime"), FadeInTime);
	
	const TArray< TSharedPtr<FJsonValue> >*  PartGroupsArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("Groups"), PartGroupsArrayObject), "Failed to get Groups field.");
	{
		for (const TSharedPtr<FJsonValue>& PartGroupValue : *PartGroupsArrayObject)
		{
			FCubismPosePartGroup PartGroup;

			TArray< TSharedPtr<FJsonValue> > PartArrayObject = PartGroupValue->AsArray();
			for (const TSharedPtr<FJsonValue>& PartValue : PartArrayObject)
			{
				FCubismPosePart Part;

				const TSharedPtr<FJsonObject>& PartObject = PartValue->AsObject();

				ASSERT(PartObject->TryGetStringField(TEXT("Id"), Part.Id), "Failed to get Id field.");

				const TArray< TSharedPtr<FJsonValue> >*  LinksArrayObject;
				ASSERT(PartObject->TryGetArrayField(TEXT("Link"), LinksArrayObject), "Failed to get Link field.");
				{
					for (const TSharedPtr<FJsonValue>& LinkValue : *LinksArrayObject)
					{
						Part.Links.Add(LinkValue->AsString());
					}
				}

				PartGroup.Parts.Add(Part);
			}

			PartGroups.Add(PartGroup);
		}
	}

	return true;
}

void FCubismPose3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismPose3Json>& Json)
{
	Json->Version = Version;
	Json->FadeInTime = FadeInTime;
	
	Json->PartGroups = PartGroups;
}
