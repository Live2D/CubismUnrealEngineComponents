/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismParameterStoreComponent.h"

#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"

UCubismParameterStoreComponent::UCubismParameterStoreComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismParameterStoreComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	ParameterValues.Empty();

	PartOpacities.Empty();

	SaveParameters();

	Model->ParameterStore = this;
}

void UCubismParameterStoreComponent::SaveParameterValue(const int32 ParameterIndex)
{
	ParameterValues.Add(ParameterIndex, Model->GetParameter(ParameterIndex)->GetParameterValue());
}

void UCubismParameterStoreComponent::SavePartOpacity(const int32 PartIndex)
{
	PartOpacities.Add(PartIndex, Model->GetPart(PartIndex)->Opacity);
}

void UCubismParameterStoreComponent::SaveParameters()
{
	for (const TObjectPtr<UCubismParameterComponent>& Parameter : Model->Parameters)
	{
		if (ParameterValues.Contains(Parameter->Index))
		{
			ParameterValues[Parameter->Index] = Parameter->GetParameterValue();
		}
		else
		{
			ParameterValues.Add(Parameter->Index, Parameter->GetParameterValue());
		}
	}

	for (const TObjectPtr<UCubismPartComponent>& Part : Model->Parts)
	{
		if (PartOpacities.Contains(Part->Index))
		{
			PartOpacities[Part->Index] = Part->Opacity;
		}
		else
		{
			PartOpacities.Add(Part->Index, Part->Opacity);
		}
	}
}

void UCubismParameterStoreComponent::LoadParameters()
{
	for (const TObjectPtr<UCubismParameterComponent>& Parameter : Model->Parameters)
	{
		if (ParameterValues.Contains(Parameter->Index))
		{
			Parameter->SetParameterValue(ParameterValues[Parameter->Index]);
		}
		else
		{
			ParameterValues.Add(Parameter->Index, Parameter->GetParameterValue());
		}
	}

	for (const TObjectPtr<UCubismPartComponent>& Part : Model->Parts)
	{
		if (PartOpacities.Contains(Part->Index))
		{
			Part->SetPartOpacity(PartOpacities[Part->Index]);
		}
		else
		{
			PartOpacities.Add(Part->Index, Part->Opacity);
		}
	}
}

// UObject interface
void UCubismParameterStoreComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
// End of UObject interface

// UActorComponent interface
void UCubismParameterStoreComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismParameterStoreComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model->ParameterStore == this)
	{
		Model->ParameterStore = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UCubismParameterStoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LoadParameters();
}
// End of UActorComponent interface
