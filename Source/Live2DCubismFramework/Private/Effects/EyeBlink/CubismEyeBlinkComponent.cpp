/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/EyeBlink/CubismEyeBlinkComponent.h"

#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismModel3Json.h"
#include "Model/CubismParameterComponent.h"

UCubismEyeBlinkComponent::UCubismEyeBlinkComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismEyeBlinkComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Phase = ECubismEyeBlinkPhase::Idle;
	Time = 0.0f;
	StartTime = Mean + FMath::FRandRange(-MaximumDeviation, MaximumDeviation);

	if (Json)
	{
		Ids.Empty();

		Ids.Append(Json->EyeBlinks);
	}

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	AddTickPrerequisiteComponent(Model->Motion); // must be updated at first because motions overwrite parameters
	AddTickPrerequisiteComponent(Model->Pose); // must be updated at first because poses overwrite parameters
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

// UObject interface
void UCubismEyeBlinkComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismEyeBlinkComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismEyeBlinkComponent, Value))
	{
		for (const FString& Id : Ids)
		{
			UCubismParameterComponent* Destination = Model->GetParameter(Id);

			if (!Destination)
			{
				continue;
			}

			switch (BlendMode)
			{
				case ECubismParameterBlendMode::Overwrite:
				{
					Destination->SetParameterValue(Value);
					break;
				}
				case ECubismParameterBlendMode::Additive:
				{
					Destination->AddParameterValue(Value);
					break;
				}
				case ECubismParameterBlendMode::Multiplicative:
				{
					Destination->MultiplyParameterValue(Value);
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
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismEyeBlinkComponent, bAutoEnabled))
	{
		Time = 0.0f;
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismEyeBlinkComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismEyeBlinkComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Update(DeltaTime);

	for (const FString& Id : Ids)
	{
		UCubismParameterComponent* Destination = Model->GetParameter(Id);

		if (!Destination)
		{
			continue;
		}

		switch (BlendMode)
		{
			case ECubismParameterBlendMode::Overwrite:
			{
				Destination->SetParameterValue(Value);
				break;
			}
			case ECubismParameterBlendMode::Additive:
			{
				Destination->AddParameterValue(Value);
				break;
			}
			case ECubismParameterBlendMode::Multiplicative:
			{
				Destination->MultiplyParameterValue(Value);
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

void UCubismEyeBlinkComponent::Update(const float DeltaTime)
{
	if (!bAutoEnabled)
	{
		return;
	}

	Time += DeltaTime;

	const float ElapsedTime = TimeScale * (Time - StartTime);

	float NewValue = 0.0f;

	switch (Phase)
	{
		case ECubismEyeBlinkPhase::Idle:
		{
			if (Time >= StartTime)
			{
				Phase = ECubismEyeBlinkPhase::Closing;
				StartTime = Time;
			}

			NewValue = 1.0f;

			break;
		}
		case ECubismEyeBlinkPhase::Closing:
		{
			float t = ElapsedTime / ClosingPeriod;

			if (t >= 1.0f)
			{
				t = 1.0f;
				Phase = ECubismEyeBlinkPhase::Closed;
				StartTime = Time;
			}

			NewValue = 1.0f - t;

			break;
		}
		case ECubismEyeBlinkPhase::Closed:
		{
			float t = ElapsedTime / ClosedPeriod;

			if (t >= 1.0f)
			{
				t = 1.0f;
				Phase = ECubismEyeBlinkPhase::Opening;
				StartTime = Time;
			}

			NewValue = 0.0f;

			break;
		}
		case ECubismEyeBlinkPhase::Opening:
		{
			float t = ElapsedTime / OpeningPeriod;

			if (t >= 1.0f)
			{
				t = 1.0f;
				Phase = ECubismEyeBlinkPhase::Idle;
				Time = 0.0f;
				StartTime = Mean + FMath::FRandRange(-MaximumDeviation, MaximumDeviation);
			}

			NewValue = t;

			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}

	Value = NewValue;
}
