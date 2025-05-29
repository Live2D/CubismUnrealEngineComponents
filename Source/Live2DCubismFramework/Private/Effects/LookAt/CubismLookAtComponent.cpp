/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/LookAt/CubismLookAtComponent.h"

#include "Effects/LookAt/CubismLookAtParameter.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismParameterComponent.h"

UCubismLookAtComponent::UCubismLookAtComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismLookAtComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		return;
	}

	if (Model != InModel)
	{
		Model = InModel;
	}

	LastPosition = FVector::ZeroVector;
	CurrentVelocity = FVector::ZeroVector;

	if (Model->LookAt != this)
	{
		if (Model->LookAt)
		{
			Model->LookAt->DestroyComponent();
		}
		Model->LookAt = this;
	}

	Model->AddTickPrerequisiteComponent(this); // model ticks after parameters are updated by components
}

// UObject interface
void UCubismLookAtComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
// End of UObject interface

// UActorComponent interface
void UCubismLookAtComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismLookAtComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model && Model->LookAt == this)
	{
		Model->LookAt = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismLookAtComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismLookAtComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Model)
	{
		return;
	}

	LastPosition = SmoothDamp(LastPosition, DeltaTime);

	for (FCubismLookAtParameter& Parameter : Parameters)
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

		switch (Parameter.Axis)
		{
		case ECubismLookAtAxis::X:
		{
			Parameter.Value = Parameter.Factor * LastPosition.X;
			break;
		}
		case ECubismLookAtAxis::Y:
		{
			Parameter.Value = Parameter.Factor * LastPosition.Y;
			break;
		}
		case ECubismLookAtAxis::Z:
		{
			Parameter.Value = Parameter.Factor * LastPosition.Z;
			break;
		}
		default:
		{
			ensure(false);
			break;
		}
		}

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

FVector UCubismLookAtComponent::SmoothDamp(const FVector CurrentValue, const float DeltaTime)
{
	// global(world) coordinates to local(object) coordinates
	const FTransform Transform = Model->GetRelativeTransform();
	FVector TargetValue = Transform.InverseTransformPosition(Target ? Target->GetActorLocation() : Transform.GetLocation());

	const float Scale = 100.0f / Model->GetPixelsPerUnit();
	TargetValue = FVector(-TargetValue.Y, TargetValue.Z, TargetValue.X) * Scale;

	const float Omega = 2.0f / Smoothing;
	const float x = Omega * DeltaTime;
	const float Invexp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

	const FVector Damp = CurrentValue - TargetValue;

	const FVector Tmp = (CurrentVelocity + Omega * Damp) * DeltaTime;
	CurrentVelocity = (CurrentVelocity - Omega * Tmp) * Invexp;

	FVector NewDamp = (Damp + Tmp) * Invexp;

	if (FVector::DotProduct(Damp, NewDamp) < 0.0f)
	{
		NewDamp = FVector::ZeroVector;
		CurrentVelocity = FVector::ZeroVector;
	}

	return NewDamp + TargetValue;
}
