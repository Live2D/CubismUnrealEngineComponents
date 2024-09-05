/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/HarmonicMotion/CubismHarmonicMotionComponent.h"

#include "Effects/HarmonicMotion/CubismHarmonicMotionParameter.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismParameterComponent.h"

UCubismHarmonicMotionComponent::UCubismHarmonicMotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismHarmonicMotionComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	AddTickPrerequisiteComponent(Model->Motion); // must be updated at first because motions overwrite parameters
	AddTickPrerequisiteComponent(Model->Pose); // must be updated at first because poses overwrite parameters
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

// UObject interface
void UCubismHarmonicMotionComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
// End of UObject interface

// UActorComponent interface
void UCubismHarmonicMotionComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismHarmonicMotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (FCubismHarmonicMotionParameter& Parameter : Parameters)
	{
		if (!Parameter.bEnabled)
		{
			continue;
		}

		UCubismParameterComponent* Destination = Model->GetParameter(Parameter.Id);

		if (!Destination)
		{
			continue;
		}

		Time += DeltaTime * Parameter.TimeScale;
		Parameter.Value = Parameter.CalcValue(Time, Destination->MinimumValue, Destination->MaximumValue);

		switch (Parameter.BlendMode)
		{
			case ECubismParameterBlendMode::Overwrite:
			{
				Destination->SetParameterValue(Parameter.Value);
				break;
			}
			case ECubismParameterBlendMode::Additive:
			{
				Destination->AddParameterValue(Parameter.Value);
				break;
			}
			case ECubismParameterBlendMode::Multiplicative:
			{
				Destination->MultiplyParameterValue(Parameter.Value);
				break;
			}
			default:
			{
				ensure(false);
				break;
			}
		}
	}
}
// End of UActorComponent interface
