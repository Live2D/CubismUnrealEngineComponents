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
	CreationMethod = EComponentCreationMethod::Instance;
}

void UCubismParameterComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);
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
		Value = (Model->GetParameterValue(Index) - MinimumValue) / (MaximumValue - MinimumValue);
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

	check(!TMathUtil<float>::IsNaN(MaximumValue));
	check(!TMathUtil<float>::IsNaN(MinimumValue));
	check(MaximumValue > MinimumValue);
}

float UCubismParameterComponent::GetParameterValue() const
{
	return Model->GetParameterValue(Index);
}

void UCubismParameterComponent::SetParameterValue(float TargetValue, const float Weight)
{
	if (!TMathUtil<float>::IsNaN(MinimumValue) && !TMathUtil<float>::IsNaN(MaximumValue))
	{
		TargetValue = FMath::Clamp(TargetValue, MinimumValue, MaximumValue);
	}

	float CurrentValue = Weight == 1.0f? TargetValue : Model->GetParameterValue(Index) * (1.0f - Weight) + TargetValue * Weight;

	Value = (CurrentValue - MinimumValue) / (MaximumValue - MinimumValue);

	Model->SetParameterValue(Index, CurrentValue);
}

void UCubismParameterComponent::AddParameterValue(float TargetValue, const float Weight)
{
	float CurrentValue = Model->GetParameterValue(Index) + TargetValue * Weight;

	if (!TMathUtil<float>::IsNaN(MinimumValue) && !TMathUtil<float>::IsNaN(MaximumValue))
	{
		CurrentValue = FMath::Clamp(CurrentValue, MinimumValue, MaximumValue);
	}

	Value = (CurrentValue - MinimumValue) / (MaximumValue - MinimumValue);

	Model->SetParameterValue(Index, CurrentValue);
}

void UCubismParameterComponent::MultiplyParameterValue(float TargetValue, const float Weight)
{
	float CurrentValue = Model->GetParameterValue(Index) * (1.0f + (TargetValue - 1.0f) * Weight);

	if (!TMathUtil<float>::IsNaN(MinimumValue) && !TMathUtil<float>::IsNaN(MaximumValue))
	{
		CurrentValue = FMath::Clamp(CurrentValue, MinimumValue, MaximumValue);
	}

	Value = (CurrentValue - MinimumValue) / (MaximumValue - MinimumValue);

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
		const float CurrentValue = (MaximumValue - MinimumValue) * Value + MinimumValue;

		Model->SetParameterValue(Index, CurrentValue);
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
// End of UActorComponent interface
