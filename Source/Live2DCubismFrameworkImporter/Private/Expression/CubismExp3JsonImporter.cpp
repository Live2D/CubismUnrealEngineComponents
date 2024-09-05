/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Expression/CubismExp3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismExp3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
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

bool FCubismExp3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	ASSERT(JsonObject->TryGetStringField(TEXT("Type"), Type), "Failed to get Type field.");
	JsonObject->TryGetNumberField(TEXT("FadeInTime"), FadeInTime);
	JsonObject->TryGetNumberField(TEXT("FadeOutTime"), FadeOutTime);

	const TArray< TSharedPtr<FJsonValue> >*  ParametersArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("Parameters"), ParametersArrayObject), "Failed to get Parameters field.");
	{
		for (const TSharedPtr<FJsonValue>& ParameterValue : *ParametersArrayObject)
		{
			FCubismExpressionParameter Parameter;

			const TSharedPtr<FJsonObject>& ParameterObject = ParameterValue->AsObject();

			ASSERT(ParameterObject->TryGetStringField(TEXT("Id"), Parameter.Id), "Failed to get Id field.");
			ASSERT(ParameterObject->TryGetNumberField(TEXT("Value"), Parameter.Value), "Failed to get Value field.");

			FString Blend;
			ASSERT(ParameterObject->TryGetStringField(TEXT("Blend"), Blend), "Failed to get Blend field.");
			
			if (Blend == TEXT("Add"))
			{
				Parameter.Blend = ECubismParameterBlendMode::Additive;
			}
			else if (Blend == TEXT("Multiply"))
			{
				Parameter.Blend = ECubismParameterBlendMode::Multiplicative;
			}
			else if (Blend == TEXT("Overwrite"))
			{
				Parameter.Blend = ECubismParameterBlendMode::Overwrite;
			}
			else
			{
				ASSERT(false, "Invalid Blend value.");
			}

			Parameters.Add(Parameter);
		}
	}

	return true;
}

void FCubismExp3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismExp3Json>& Json)
{
	Json->Type = Type;
	Json->FadeInTime = FadeInTime;
	Json->FadeOutTime = FadeOutTime;
	Json->Parameters = Parameters;
}
