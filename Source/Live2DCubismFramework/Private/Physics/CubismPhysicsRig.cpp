/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Physics/CubismPhysicsRig.h"

#include "Model/CubismParameterComponent.h"

#include "CubismMath.h"


float GetRangeValue(float Min, float Max)
{
	float MaxValue = FMath::Max(Min, Max);
	float MinValue = FMath::Min(Min, Max);

	return FMath::Abs(MaxValue - MinValue);
}

float GetDefaultValue(float Min, float Max)
{
	const float MinValue = FMath::Min(Min, Max);
	return MinValue + (GetRangeValue(Min, Max) / 2.0f);
}

int Sign(float Value)
{
	int Ret = 0;

	if (Value > 0.0f)
	{
		Ret = 1;
	}
	else if (Value < 0.0f)
	{
		Ret = -1;
	}

	return Ret;
}

float NormalizeParameterValue(
	float Value, const float ParameterMinimum, const float ParameterMaximum, const float ParameterDefault,
	const float NormalizedMinimum, const float NormalizedMaximum, const float NormalizedDefault, const bool bIsInverted
)
{
	check(ParameterMinimum <= ParameterMaximum);
	check(NormalizedMinimum <= NormalizedMaximum);

	const float MaxValue = FMath::Max(ParameterMaximum, ParameterMinimum);
	if (MaxValue < Value)
	{
		Value = MaxValue;
	}

	const float MinValue = FMath::Min(ParameterMaximum, ParameterMinimum);
	if (MinValue > Value)
	{
		Value = MinValue;
	}

	const float MinNormValue = FMath::Min(NormalizedMinimum, NormalizedMaximum);
	const float MaxNormValue = FMath::Max(NormalizedMinimum, NormalizedMaximum);
	const float MiddleNormValue = NormalizedDefault;

	const float MiddleValue = GetDefaultValue(MinValue, MaxValue);
	const float ParamValue = Value - MiddleValue;

	float Result = 0.0f;

	switch (Sign(ParamValue))
	{
	case 1: {
		const float NLength = MaxNormValue - MiddleNormValue;
		const float PLength = MaxValue - MiddleValue;
		if (PLength != 0.0f)
		{
			Result = ParamValue * (NLength / PLength);
			Result += MiddleNormValue;
		}
		break;
	}
	case -1: {
		const float NLength = MinNormValue - MiddleNormValue;
		const float PLength = MinValue - MiddleValue;
		if (PLength != 0.0f)
		{
			Result = ParamValue * (NLength / PLength);
			Result += MiddleNormValue;
		}
		break;
	}
	case 0: {
		Result = MiddleNormValue;
		break;
	}
	default: {
		break;
	}
	}

	return bIsInverted? -Result : Result;
}

void FCubismPhysicsRigInput::GetNormalizedParameterValue(
	FVector2D& TargetTranslation,
	float& TargetAngle,
	const float Value,
	const FCubismPhysicsNormalization& NormalizationPosition,
	const FCubismPhysicsNormalization& NormalizationAngle
)
{
	switch (Type)
	{
		case ECubismPhysicsSource::X:
		{
			TargetTranslation.X += NormalizeParameterValue(
				Value,
				Parameter->MinimumValue,
				Parameter->MaximumValue,
				Parameter->DefaultValue,
				NormalizationPosition.Minimum,
				NormalizationPosition.Maximum,
				NormalizationPosition.Default,
				bReflect
			) * Weight;

			break;
		}
		case ECubismPhysicsSource::Y:
		{
			TargetTranslation.Y += NormalizeParameterValue(
				Value,
				Parameter->MinimumValue,
				Parameter->MaximumValue,
				Parameter->DefaultValue,
				NormalizationPosition.Minimum,
				NormalizationPosition.Maximum,
				NormalizationPosition.Default,
				bReflect
			) * Weight;

			break;
		}
		case ECubismPhysicsSource::Angle:
		{
			TargetAngle += NormalizeParameterValue(
				Value,
				Parameter->MinimumValue,
				Parameter->MaximumValue,
				Parameter->DefaultValue,
				NormalizationAngle.Minimum,
				NormalizationAngle.Maximum,
				NormalizationAngle.Default,
				bReflect
			) * Weight;

			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}
}

void FCubismPhysicsRigOutput::UpdateOutputParameterValue(float& ParameterValue, float Value)
{
	check(Parameter);

	switch (Type)
	{
		case ECubismPhysicsSource::X:
		{
			Value *= TranslationScale.X;

			break;
		}
		case ECubismPhysicsSource::Y:
		{
			Value *= TranslationScale.Y;

			break;
		}
		case ECubismPhysicsSource::Angle:
		{
			Value *= AngleScale;

			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}

	if (Value < Parameter->MinimumValue)
	{
		if (Value < ValueBelowMinimum)
		{
			ValueBelowMinimum = Value;
		}

		Value = Parameter->MinimumValue;
	}
	else if (Value > Parameter->MaximumValue)
	{
		if (Value > ValueExceededMaximum)
		{
			ValueExceededMaximum = Value;
		}

		Value = Parameter->MaximumValue;
	}

	if (Weight >= 1.0f)
	{
		ParameterValue = Value;
	}
	else
	{
		Value = (ParameterValue * (1.0f - Weight)) + (Value * Weight);
		ParameterValue = Value;
	}

	check(Parameter->MinimumValue <= ParameterValue && ParameterValue <= Parameter->MaximumValue);
}

float FCubismPhysicsRigOutput::GetValue(const TArray<struct FCubismPhysicsRigParticle> Particles, const FVector2D ParentGravity) const
{
	float Value = 0.0f;

	const FVector2D Translation = Particles[ParticleIndex].Position - Particles[ParticleIndex - 1].Position;

	switch (Type)
	{
		case ECubismPhysicsSource::X:
		{
			Value = bReflect? -Translation.X : Translation.X;

			break;
		}
		case ECubismPhysicsSource::Y:
		{
			Value = bReflect? -Translation.Y : Translation.Y;

			break;
		}
		case ECubismPhysicsSource::Angle:
		{
			const FVector2D Gravity = ParticleIndex < 2? -ParentGravity : Particles[ParticleIndex - 1].Position - Particles[ParticleIndex - 2].Position;

			Value = FCubismMath::DirectionToRadian(Gravity, Translation);

			if (bReflect)
			{
				Value *= -1.0f;
			}

			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}

	return Value;
}
