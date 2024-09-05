/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "DisplayInfo/CubismDisplayInfo3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismDisplayInfo3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
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

bool FCubismDisplayInfo3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	ASSERT(JsonObject->TryGetNumberField(TEXT("Version"), Version), "Failed to get Version field.");

	const TArray< TSharedPtr<FJsonValue> >* ParametersArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("Parameters"), ParametersArrayObject), "Failed to get Parameters field.");
	for (const TSharedPtr<FJsonValue>& ParameterValue : *ParametersArrayObject)
	{
		const TSharedPtr<FJsonObject>& ParameterObject = ParameterValue->AsObject();

		FCubismDisplayInfoParameter Parameter;

		ASSERT(ParameterObject->TryGetStringField(TEXT("Id"), Parameter.Id), "Failed to get Id field.");
		ASSERT(ParameterObject->TryGetStringField(TEXT("GroupId"), Parameter.GroupId), "Failed to get GroupId field.");

		FString NameString;

		ASSERT(ParameterObject->TryGetStringField(TEXT("Name"), NameString), "Failed to get Name field.");

		Parameter.Name = FText::FromString(NameString);

		Parameters.Add(Parameter);
	}

	const TArray< TSharedPtr<FJsonValue> >* ParameterGroupsArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("ParameterGroups"), ParameterGroupsArrayObject), "Failed to get ParameterGroups field.");
	for (const TSharedPtr<FJsonValue>& ParameterGroupValue : *ParameterGroupsArrayObject)
	{
		FCubismDisplayInfoParameterGroup ParameterGroup;

		const TSharedPtr<FJsonObject>& ParameterGroupObject = ParameterGroupValue->AsObject();

		ASSERT(ParameterGroupObject->TryGetStringField(TEXT("Id"), ParameterGroup.Id), "Failed to get Id field.");
		ASSERT(ParameterGroupObject->TryGetStringField(TEXT("GroupId"), ParameterGroup.GroupId), "Failed to get GroupId field.");

		FString NameString;

		ASSERT(ParameterGroupObject->TryGetStringField(TEXT("Name"), NameString), "Failed to get Name field.");

		ParameterGroup.Name = FText::FromString(NameString);

		ParameterGroups.Add(ParameterGroup);
	}

	const TArray< TSharedPtr<FJsonValue> >* PartsArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("Parts"), PartsArrayObject), "Failed to get Parts field.");
	for (const TSharedPtr<FJsonValue>& PartValue : *PartsArrayObject)
	{
		FCubismDisplayInfoPart Part;

		const TSharedPtr<FJsonObject>& PartObject = PartValue->AsObject();

		ASSERT(PartObject->TryGetStringField(TEXT("Id"), Part.Id), "Failed to get Id field.");

		FString NameString;

		ASSERT(PartObject->TryGetStringField(TEXT("Name"), NameString), "Failed to get Name field.");

		Part.Name = FText::FromString(NameString);

		Parts.Add(Part);
	}

	return true;
}

void FCubismDisplayInfo3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismDisplayInfo3Json>& Json)
{
	Json->Parameters = Parameters;
	Json->ParameterGroups = ParameterGroups;
	Json->Parts = Parts;
}
