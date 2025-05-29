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

	check(Index >= 0 && Index < InModel->GetParameterCount() || InModel->NonNativeParameterIds.Contains(Index));

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
	float CurrentValue = Weight == 1.0f? TargetValue : Model->GetParameterValue(Index) * (1.0f - Weight) + TargetValue * Weight;

	if (!FGenericPlatformMath::IsNaN(MinimumValue) && !FGenericPlatformMath::IsNaN(MaximumValue))
	{
		CurrentValue = FMath::Clamp(CurrentValue, MinimumValue, MaximumValue);
	}

	Value = CurrentValue;

	Model->SetParameterValue(Index, CurrentValue);
}

void UCubismParameterComponent::AddParameterValue(float TargetValue, const float Weight)
{
	float CurrentValue = Model->GetParameterValue(Index) + TargetValue * Weight;

	if (!FGenericPlatformMath::IsNaN(MinimumValue) && !FGenericPlatformMath::IsNaN(MaximumValue))
	{
		CurrentValue = FMath::Clamp(CurrentValue, MinimumValue, MaximumValue);
	}

	Value = CurrentValue;

	Model->SetParameterValue(Index, CurrentValue);
}

void UCubismParameterComponent::MultiplyParameterValue(float TargetValue, const float Weight)
{
	float CurrentValue = Model->GetParameterValue(Index) * (1.0f + (TargetValue - 1.0f) * Weight);

	if (!FGenericPlatformMath::IsNaN(MinimumValue) && !FGenericPlatformMath::IsNaN(MaximumValue))
	{
		CurrentValue = FMath::Clamp(CurrentValue, MinimumValue, MaximumValue);
	}

	Value = CurrentValue;

	Model->SetParameterValue(Index, CurrentValue);
}

// UObject interface
void UCubismParameterComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismParameterComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismParameterComponent, Value))
	{
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

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismParameterComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif
// End of UActorComponent interface
