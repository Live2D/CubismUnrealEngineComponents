/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "UserData/CubismUserData3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismUserData3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
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

bool FCubismUserData3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	ASSERT(JsonObject->TryGetNumberField(TEXT("Version"), Version), "Failed to get Version field.");

	const TSharedPtr<FJsonObject>* Meta;
	ASSERT(JsonObject->TryGetObjectField(TEXT("Meta"), Meta), "Failed to get Meta field.");
	{
		ASSERT((*Meta)->TryGetNumberField(TEXT("UserDataCount"), Size), "Failed to get UserDataCount field.");
	}

	const TArray< TSharedPtr<FJsonValue> >*  UserDataArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("UserData"), UserDataArrayObject), "Failed to get UserData field.");
	for (const TSharedPtr<FJsonValue>& UserDataValue : *UserDataArrayObject)
	{
		const TSharedPtr<FJsonObject>& UserDataObject = UserDataValue->AsObject();

		FString TargetValue;
		ASSERT(UserDataObject->TryGetStringField(TEXT("Target"), TargetValue), "Failed to get Target field.");

		ECubismUserDataTargetType Target = ECubismUserDataTargetType::ArtMesh;
		if (TargetValue == TEXT("ArtMesh"))
		{
			Target = ECubismUserDataTargetType::ArtMesh;
		}
		else
		{
			check(false);
		}

		if (!Data.Contains(Target))
		{
			FCubismUserDataEntry Entry;
			Data.Add(Target, Entry);
		}

		FCubismUserDataEntry& Entry = Data[Target];

		FString Id, Value;
		ASSERT(UserDataObject->TryGetStringField(TEXT("Id"), Id), "Failed to get Id field.");
		ASSERT(UserDataObject->TryGetStringField(TEXT("Value"), Value), "Failed to get Value field.");

		Entry.Tags.Add(Id, Value);
	}

	return true;
}

void FCubismUserData3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismUserData3Json>& Json)
{
	Json->Size = Size;
	Json->Data = Data;
}
