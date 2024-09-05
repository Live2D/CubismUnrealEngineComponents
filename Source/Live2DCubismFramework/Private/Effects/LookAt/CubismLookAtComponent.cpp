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
#include "Kismet/GameplayStatics.h"

const float FrameRate = 30.0f;
const float Epsilon = 0.01f;

UCubismLookAtComponent::UCubismLookAtComponent()
	: FaceTargetX(0.0f)
	, FaceTargetY(0.0f)
	, FaceX(0.0f)
	, FaceY(0.0f)
	, FaceVX(0.0f)
	, FaceVY(0.0f)
	, LastTimeSeconds(0.0f)
	, UserTimeSeconds(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismLookAtComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	LastPosition = FVector::ZeroVector;
	CurrentVelocity = FVector::ZeroVector;

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	AddTickPrerequisiteComponent(Model->Motion); // must be updated at first because motions overwrite parameters
	AddTickPrerequisiteComponent(Model->Pose); // must be updated at first because poses overwrite parameters
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
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

void UCubismLookAtComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

		switch(Parameter.BlendMode)
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

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return CurrentValue;
	}

	FVector2D ScreenPosition;
	PlayerController->ProjectWorldLocationToScreen((Target->GetActorLocation() - Transform.GetLocation()), ScreenPosition, false);

	FVector2D Resolution = GEngine->GameViewport->Viewport->GetSizeXY();

	FaceTargetX = (float)(ScreenPosition.X / Resolution.X) * 2.0f - 1.0f;
	FaceTargetY = (float)(ScreenPosition.Y / Resolution.Y) * 2.0f - 1.0f;

	UserTimeSeconds += DeltaTime;

	const float FaceParamMaxV = 40.0f / 10.0f;
	const float MaxV = FaceParamMaxV * 1.0f / FrameRate;


	if (LastTimeSeconds == 0.0f)
	{
		LastTimeSeconds = UserTimeSeconds;
		return CurrentValue;
	}


	const float DeltaTimeWeight = (UserTimeSeconds - LastTimeSeconds) * FrameRate;
	LastTimeSeconds = UserTimeSeconds;

	const float TimeToMaxSpeed = 0.15f;
	const float FrameToMaxSpeed = TimeToMaxSpeed * FrameRate;     // sec * frame/sec
	const float MaxA = DeltaTimeWeight * MaxV / FrameToMaxSpeed;


	const float DX = FaceTargetX - FaceX;
	const float DY = FaceTargetY - FaceY;


	if (FMath::Abs(DX) <= Epsilon && FMath::Abs(DY) <= Epsilon)
	{
		return CurrentValue;
	}

	const float D = FMath::Sqrt((DX * DX) + (DY * DY));


	const float VX = MaxV * DX / D;
	const float VY = MaxV * DY / D;


	float AX = VX - FaceVX;
	float AY = VY - FaceVY;

	const float A = FMath::Sqrt((AX * AX) + (AY * AY));


	if (A < -MaxA || A > MaxA)
	{
		AX *= MaxA / A;
		AY *= MaxA / A;
	}

	FaceVX += AX;
	FaceVY += AY;

	{

		const float MaxVelocity = 0.5f * (FMath::Sqrt((MaxA * MaxA) + 16.0f * MaxA * D - 8.0f * MaxA * D) - MaxA);
		const float CurVelocity = FMath::Sqrt((FaceVX * FaceVX) + (FaceVY * FaceVY));

		if (CurVelocity > MaxVelocity)
		{
			FaceVX *= MaxVelocity / CurVelocity;
			FaceVY *= MaxVelocity / CurVelocity;
		}
	}

	FaceX += FaceVX;
	FaceY += FaceVY;

	FVector Ret = FVector(-FaceX, -FaceY, 0);


	return Ret;
}
