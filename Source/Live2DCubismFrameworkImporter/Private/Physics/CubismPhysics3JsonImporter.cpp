/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Physics/CubismPhysics3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismPhysics3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
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

bool FCubismPhysics3JsonImporter::ImportFromString(const FString& FileContent)
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
		ASSERT((*Meta)->TryGetNumberField(TEXT("PhysicsSettingCount"), TotalInputCount), "Failed to get PhysicsSettingCount field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("TotalInputCount"), TotalInputCount), "Failed to get TotalInputCount field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("TotalOutputCount"), TotalOutputCount), "Failed to get TotalOutputCount field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("VertexCount"), TotalParticleCount), "Failed to get VertexCount field.");

		(*Meta)->TryGetNumberField(TEXT("Fps"), Fps);

		const TSharedPtr<FJsonObject>* EffectiveForces;
		ASSERT((*Meta)->TryGetObjectField(TEXT("EffectiveForces"), EffectiveForces), "Failed to get EffectiveForces field.");
		{
			const TSharedPtr<FJsonObject>* GravityObject;
			if ((*EffectiveForces)->TryGetObjectField(TEXT("Gravity"), GravityObject))
			{
				(*GravityObject)->TryGetNumberField(TEXT("X"), Gravity.X);
				(*GravityObject)->TryGetNumberField(TEXT("Y"), Gravity.Y);
			}

			const TSharedPtr<FJsonObject>* WindObject;
			if ((*EffectiveForces)->TryGetObjectField(TEXT("Wind"), WindObject))
			{
				(*WindObject)->TryGetNumberField(TEXT("X"), Wind.X);
				(*WindObject)->TryGetNumberField(TEXT("Y"), Wind.Y);
			}
		}

		const TArray< TSharedPtr<FJsonValue> >* PhysicsDictionaryArrayObject;
		ASSERT((*Meta)->TryGetArrayField(TEXT("PhysicsDictionary"), PhysicsDictionaryArrayObject), "Failed to get PhysicsDictionary field.");
		for (const TSharedPtr<FJsonValue>& PhysicsDictionaryValue : *PhysicsDictionaryArrayObject)
		{
			const TSharedPtr<FJsonObject>& PhysicsDictionaryObject = PhysicsDictionaryValue->AsObject();

			FString Id, Name;

			ASSERT(PhysicsDictionaryObject->TryGetStringField(TEXT("Id"), Id), "Failed to get Id field.");
			ASSERT(PhysicsDictionaryObject->TryGetStringField(TEXT("Name"), Name), "Failed to get Name field.");

			PhysicsSettingNameMap.Add(Id, FText::FromString(Name));
		}
	}

	const TArray< TSharedPtr<FJsonValue> >* PhysicsSettingsArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("PhysicsSettings"), PhysicsSettingsArrayObject), "Failed to get PhysicsSettings field.");
	for (const TSharedPtr<FJsonValue>& PhysicsSettingValue : *PhysicsSettingsArrayObject)
	{
		const TSharedPtr<FJsonObject>& PhysicsSettingObject = PhysicsSettingValue->AsObject();

		FCubismPhysicsSetting PhysicsSetting;

		ASSERT(PhysicsSettingObject->TryGetStringField(TEXT("Id"), PhysicsSetting.Id), "Failed to get Id field.");

		if (PhysicsSettingNameMap.Contains(PhysicsSetting.Id))
		{
			PhysicsSetting.Name = PhysicsSettingNameMap[PhysicsSetting.Id];
		}

		const TArray< TSharedPtr<FJsonValue> >* InputArrayObject;
		ASSERT(PhysicsSettingObject->TryGetArrayField(TEXT("Input"), InputArrayObject), "Failed to get Input field.");
		for (const TSharedPtr<FJsonValue>& InputValue : *InputArrayObject)
		{
			const TSharedPtr<FJsonObject>& InputObject = InputValue->AsObject();

			FCubismPhysicsInput Input;

			const TSharedPtr<FJsonObject>* Source;
			ASSERT(InputObject->TryGetObjectField(TEXT("Source"), Source), "Failed to get Source field.");
			{
				ASSERT((*Source)->TryGetStringField(TEXT("Id"), Input.Source.Id), "Failed to get Id field.");

				FString TargetString;
				ASSERT((*Source)->TryGetStringField(TEXT("Target"), TargetString), "Failed to get Target field.");
				if (TargetString == TEXT("Parameter"))
				{
					Input.Source.Target = ECubismPhysicsTargetType::Parameter;
				}
			}

			ASSERT(InputObject->TryGetNumberField(TEXT("Weight"), Input.Weight), "Failed to get Weight field.");

			FString TypeString;
			ASSERT(InputObject->TryGetStringField(TEXT("Type"), TypeString), "Failed to get Type field.");
			if (TypeString == TEXT("X"))
			{
				Input.Type = ECubismPhysicsSource::X;
			}
			else if (TypeString == TEXT("Y"))
			{
				Input.Type = ECubismPhysicsSource::Y;
			}
			else if (TypeString == TEXT("Angle"))
			{
				Input.Type = ECubismPhysicsSource::Angle;
			}

			ASSERT(InputObject->TryGetBoolField(TEXT("Reflect"), Input.bReflect), "Failed to get Reflect field.");

			PhysicsSetting.Inputs.Add(Input);
		}

		const TArray< TSharedPtr<FJsonValue> >* OutputArrayObject;
		ASSERT(PhysicsSettingObject->TryGetArrayField(TEXT("Output"), OutputArrayObject), "Failed to get Output field.");
		for (const TSharedPtr<FJsonValue>& OutputValue : *OutputArrayObject)
		{
			const TSharedPtr<FJsonObject>& OutputObject = OutputValue->AsObject();

			FCubismPhysicsOutput Output;

			const TSharedPtr<FJsonObject>* Destination;
			ASSERT(OutputObject->TryGetObjectField(TEXT("Destination"), Destination), "Failed to get Destination field.");
			{
				ASSERT((*Destination)->TryGetStringField(TEXT("Id"), Output.Destination.Id), "Failed to get Id field.");

				FString TargetString;
				ASSERT((*Destination)->TryGetStringField(TEXT("Target"), TargetString), "Failed to get Target field.");
				if (TargetString == TEXT("Parameter"))
				{
					Output.Destination.Target = ECubismPhysicsTargetType::Parameter;
				}
			}

			ASSERT(OutputObject->TryGetNumberField(TEXT("VertexIndex"), Output.VertexIndex), "Failed to get VertexIndex field.");
			ASSERT(OutputObject->TryGetNumberField(TEXT("Scale"), Output.AngleScale), "Failed to get Scale field.");
			ASSERT(OutputObject->TryGetNumberField(TEXT("Weight"), Output.Weight), "Failed to get Weight field.");

			FString TypeString;
			ASSERT(OutputObject->TryGetStringField(TEXT("Type"), TypeString), "Failed to get Type field.");
			if (TypeString == TEXT("X"))
			{
				Output.Type = ECubismPhysicsSource::X;
			}
			else if (TypeString == TEXT("Y"))
			{
				Output.Type = ECubismPhysicsSource::Y;
			}
			else if (TypeString == TEXT("Angle"))
			{
				Output.Type = ECubismPhysicsSource::Angle;
			}

			ASSERT(OutputObject->TryGetBoolField(TEXT("Reflect"), Output.bReflect), "Failed to get Reflect field.");

			PhysicsSetting.Outputs.Add(Output);
		}

		const TArray< TSharedPtr<FJsonValue> >* VertexArrayObject;
		ASSERT(PhysicsSettingObject->TryGetArrayField(TEXT("Vertices"), VertexArrayObject), "Failed to get Vertices field.");
		for (const TSharedPtr<FJsonValue>& VertexValue : *VertexArrayObject)
		{
			const TSharedPtr<FJsonObject>& VertexObject = VertexValue->AsObject();

			FCubismPhysicsParticle Particle;

			ASSERT(VertexObject->TryGetNumberField(TEXT("Mobility"), Particle.Mobility), "Failed to get Mobility field.");
			ASSERT(VertexObject->TryGetNumberField(TEXT("Delay"), Particle.Delay), "Failed to get Delay field.");
			ASSERT(VertexObject->TryGetNumberField(TEXT("Acceleration"), Particle.Acceleration), "Failed to get Acceleration field.");
			ASSERT(VertexObject->TryGetNumberField(TEXT("Radius"), Particle.Radius), "Failed to get Radius field.");

			const TSharedPtr<FJsonObject>* PositionObject;
			ASSERT(VertexObject->TryGetObjectField(TEXT("Position"), PositionObject), "Failed to get Position field.");
			{
				(*PositionObject)->TryGetNumberField(TEXT("X"), Particle.Position.X);
				(*PositionObject)->TryGetNumberField(TEXT("Y"), Particle.Position.Y);
			}

			PhysicsSetting.Particles.Add(Particle);
		}

		const TSharedPtr<FJsonObject>* NormalizationObject;
		ASSERT(PhysicsSettingObject->TryGetObjectField(TEXT("Normalization"), NormalizationObject), "Failed to get Normalization field.");
		{
			const TSharedPtr<FJsonObject>* PositionObject;
			if ((*NormalizationObject)->TryGetObjectField(TEXT("Position"), PositionObject))
			{
				(*PositionObject)->TryGetNumberField(TEXT("Minimum"), PhysicsSetting.NormalizationPosition.Minimum);
				(*PositionObject)->TryGetNumberField(TEXT("Maximum"), PhysicsSetting.NormalizationPosition.Maximum);
				(*PositionObject)->TryGetNumberField(TEXT("Default"), PhysicsSetting.NormalizationPosition.Default);
			}

			const TSharedPtr<FJsonObject>* AngleObject;
			if ((*NormalizationObject)->TryGetObjectField(TEXT("Angle"), AngleObject))
			{
				(*AngleObject)->TryGetNumberField(TEXT("Minimum"), PhysicsSetting.NormalizationAngle.Minimum);
				(*AngleObject)->TryGetNumberField(TEXT("Maximum"), PhysicsSetting.NormalizationAngle.Maximum);
				(*AngleObject)->TryGetNumberField(TEXT("Default"), PhysicsSetting.NormalizationAngle.Default);
			}
		}

		PhysicsSettings.Add(PhysicsSetting);
	}

	return true;
}

void FCubismPhysics3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismPhysics3Json>& Json)
{
	Json->Version = Version;
	Json->Gravity = Gravity;
	Json->Wind = Wind;
	Json->Fps = Fps;
	Json->TotalInputCount = TotalInputCount;
	Json->TotalOutputCount = TotalOutputCount;
	Json->TotalParticleCount = TotalParticleCount;
	Json->PhysicsSettings = PhysicsSettings;
}
