/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Motion/CubismMotionComponent.h"

#include "Motion/CubismMotion3Json.h"
#include "Motion/CubismMotion.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismParameterStoreComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "CubismLog.h"

UCubismMotionComponent::UCubismMotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismMotionComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Time = 0.0f;
	MotionQueue.Empty();

	Model->Motion = this;

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

bool UCubismMotionComponent::IsFinished() const
{
	for (const TSharedPtr<FCubismMotion>& Motion : MotionQueue)
	{
		if (Motion->State != ECubismMotionState::End)
		{
			return false;
		}
	}

	return true;
}

bool UCubismMotionComponent::ReserveMotion(const ECubismMotionPriority Priority)
{
	if (Priority <= ReservedPriority || Priority <= CurrentPriority)
	{
		return false;
	}

	ReservedPriority = Priority;

	return true;
}

void UCubismMotionComponent::PlayMotion(const int32 InIndex, const float OffsetTime, const ECubismMotionPriority Priority)
{
	if (!Jsons.IsValidIndex(InIndex))
	{
		UE_LOG(LogCubism, Warning, TEXT("Motion cannot be played. Index is out of range."));

		return;
	}

	const TObjectPtr<UCubismMotion3Json>& Json = Jsons[InIndex];

	if (Priority == ReservedPriority || Priority == ECubismMotionPriority::Force)
	{
		ReservedPriority = ECubismMotionPriority::None;
	}

	CurrentPriority = Priority;

	for (const TSharedPtr<FCubismMotion>& Motion : MotionQueue)
	{
		Motion->FadeOut(Time);
	}

	TSharedPtr<FCubismMotion> NextMotion = MakeShared<FCubismMotion>(Json, OffsetTime);

	MotionQueue.Add(NextMotion);
}

void UCubismMotionComponent::StopAllMotions(const bool bForce)
{
	if (bForce)
	{
		MotionQueue.Empty();
	}
	else
	{
		for (const TSharedPtr<FCubismMotion>& Motion : MotionQueue)
		{
			Motion->FadeOut(Time);
		}
	}
}

// UObject interface
void UCubismMotionComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismMotionComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMotionComponent, Index))
	{
		if (Jsons.IsValidIndex(Index))
		{
			PlayMotion(Index, 0.0f, ECubismMotionPriority::Force);
		}
		else
		{
			StopAllMotions();
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismMotionComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismMotionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model->Motion == this)
	{
		Model->Motion = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UCubismMotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Time += Speed * DeltaTime;

	for (const TSharedPtr<FCubismMotion>& Motion : MotionQueue)
	{
		if (Motion->State == ECubismMotionState::None)
		{
			// Initialize the motion.
			Motion->Init(Time);
		}

		UpdateMotion(Motion);
	}

	Model->ParameterStore->SaveParameters();

	for (int32 i = MotionQueue.Num()-1; i >= 0; i--)
	{
		const TSharedPtr<FCubismMotion>& Motion = MotionQueue[i];

		if (Motion->State == ECubismMotionState::End)
		{
			MotionQueue.RemoveAt(i);
		}
	}

	if (IsFinished())
	{
		CurrentPriority = ECubismMotionPriority::None;

		OnMotionPlaybackFinished.Broadcast();
	}
}
// End of UActorComponent interface

void UCubismMotionComponent::UpdateMotion(const TSharedPtr<FCubismMotion>& Motion)
{
	float ElapsedTime = Time - Motion->StartTime;

	if (Motion->State == ECubismMotionState::PlayInLoop)
	{
		while (ElapsedTime > Motion->Duration)
		{
			ElapsedTime -= Motion->Duration;
		}
	}

    for (const FCubismMotionCurve& Curve : Motion->Curves)
    {
		const float Value = Motion->GetValue(Curve.Id, ElapsedTime);

        switch (Curve.Target)
        {
            case ECubismMotionCurveTarget::Model:
            {
                Model->Opacity = Value;

                break;
            }
            case ECubismMotionCurveTarget::Parameter:
            {
                if (UCubismParameterComponent* DstParameter = Model->GetParameter(Curve.Id))
                {
    				const float FadeWeight = Motion->CalcWeight(Curve.Id, ElapsedTime);

                    DstParameter->SetParameterValue(Value, FadeWeight);
                }

                break;
            }
            case ECubismMotionCurveTarget::PartOpacity:
            {
				UCubismParameterComponent* DstParameter = Model->GetParameter(Curve.Id);

				check(DstParameter);

				DstParameter->SetParameterValue(Value);

                break;
            }
            default:
            {
                ensure(false);
                break;
            }
        }
    }

	if (Time - Motion->StartTime > Motion->Duration)
	{
		if (Motion->State == ECubismMotionState::PlayInLoop)
		{
			Motion->StartTime = Time;
		}
		else
		{
			Motion->State = ECubismMotionState::End;
		}
	}
}
