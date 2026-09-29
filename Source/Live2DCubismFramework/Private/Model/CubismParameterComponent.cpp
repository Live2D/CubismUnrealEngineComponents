/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismParameterComponent.h"

#include "Model/CubismParameterStoreComponent.h"
#include "Model/CubismModelActor.h"

#include "Live2DCubismCore.h"

UCubismParameterComponent::UCubismParameterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismParameterComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		return;
	}

	if ((Index < 0 || Index >= InModel->GetParameterCount()) && !InModel->NonNativeParameterIds.Contains(Index))
	{
		return;
	}

	if (Model == InModel)
	{
		return;
	}

	Model = InModel;

	if (Index >= 0 && Index < InModel->GetParameterCount())
	{
		Id = Model->GetParameterId(Index);
		Type = Model->GetParameterType(Index);
		MaximumValue = Model->GetParameterMaximumValue(Index);
		MinimumValue = Model->GetParameterMinimumValue(Index);
		DefaultValue = Model->GetParameterDefaultValue(Index);
		Value = Model->GetParameterValue(Index);
	}
	else
	{
		Id = Model->NonNativeParameterIds[Index];
		Type = ECubismParameterType::Normal;
		MaximumValue = 1.0f;
		MinimumValue = 0.0f;
		DefaultValue = 0.0f;
		Value = 0.0f;
	}

	check(!FGenericPlatformMath::IsNaN(MaximumValue));
	check(!FGenericPlatformMath::IsNaN(MinimumValue));
	check(MaximumValue > MinimumValue);
}

void UCubismParameterComponent::SetParameterValue(float TargetValue, const float Weight)
{
	if (!Model)
	{
		return;
	}
	float CurrentValue = Weight == 1.0f
		? TargetValue
		: Value * (1.0f - Weight) + TargetValue * Weight;

	Value = CurrentValue;
	Model->SetParameterValue(Index, CurrentValue);
}

void UCubismParameterComponent::AddParameterValue(float TargetValue, const float Weight)
{
	if (!Model)
	{
		return;
	}
	SetParameterValue(Value + TargetValue, Weight);
}

void UCubismParameterComponent::MultiplyParameterValue(float TargetValue, const float Weight)
{
	SetParameterValue(Value * (1.0f + (TargetValue - 1.0f) * Weight), Weight);
}

bool UCubismParameterComponent::IsRepeat() const
{
	if (!Model)
	{
		return false;
	}

	if (Model->GetOverrideFlagForModelParameterRepeat())
	{
		return bRepeat;
	}

	if (bOverrideRepeat)
	{
		return bRepeat;
	}
	
	const unsigned char* RepeatFlags = csmGetParameterRepeats(Model->RawModel);
	return (RepeatFlags && Index >= 0) ? (RepeatFlags[Index] != 0) : false;
}

float UCubismParameterComponent::GetParameterRepeatValue(float InValue) const
{
	const float Range = MaximumValue - MinimumValue;

	if (Range <= KINDA_SMALL_NUMBER) return MinimumValue;

	float t = FMath::Fmod(InValue - MinimumValue, Range);
	if (t < 0.0f) t += Range;
	return MinimumValue + t;
}

float UCubismParameterComponent::GetParameterClampValue(float InValue) const
{
	return FMath::Clamp(InValue, MinimumValue, MaximumValue);
}

// UObject interface
void UCubismParameterComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismParameterComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismParameterComponent, Value))
	{
		if (!Model)
		{
			return;
		}
		Model->SetParameterValue(Index, Value);

		if(Model->ParameterStore)
		{
			Model->ParameterStore->SaveParameterValue(Index);
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismParameterComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismParameterComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}
#endif

void UCubismParameterComponent::OverrideValue(float InValue, float Weight)
{
	SetParameterValue(InValue, Weight);
}
// End of UActorComponent interface
