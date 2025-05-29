/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/EyeBlink/CubismEyeBlinkComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismModel3Json.h"
#include "Model/CubismParameterComponent.h"

UCubismEyeBlinkComponent::UCubismEyeBlinkComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismEyeBlinkComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ubismEyeBlinkComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

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

	if (Model->EyeBlink != this)
	{
		if (Model->EyeBlink)
		{
			Model->EyeBlink->DestroyComponent();
		}
		Model->EyeBlink = this;
	}

	Model->AddTickPrerequisiteComponent(this); // model ticks after parameters are updated by components
}

// UObject interface
void UCubismEyeBlinkComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());
	if (!Owner || !Owner->Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Owner or Model."));
		return;
	}

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

	const FName EyeBlinkPropertyName = PropertyChangedEvent.GetPropertyName();

	if (EyeBlinkPropertyName == GET_MEMBER_NAME_CHECKED(UCubismEyeBlinkComponent, bEnableEyeBlinkInEditor))
	{
		if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
		{
			SetComponentTickEnabled(bEnableEyeBlinkInEditor);
		}
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

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		SetComponentTickEnabled(bEnableEyeBlinkInEditor);
	}
#endif
}

void UCubismEyeBlinkComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model && Model->EyeBlink == this)
	{
		Model->EyeBlink = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismEyeBlinkComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismEyeBlinkComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	OnCubismUpdate(DeltaTime);
}

void UCubismEyeBlinkComponent::OnCubismUpdate(float DeltaTime)
{
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor && !bEnableEyeBlinkInEditor)
	{
		return;
	}
#endif

	if (!Model)
	{
		return;
	}

	Update(DeltaTime);

	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("EyeBlinkComponent: Model is null."));
		return;
	}

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

int32 UCubismEyeBlinkComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_EYEBLINK;
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
