/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Motion/CubismMotionComponent.h"

#include "CubismUpdateExecutionOrder.h"
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
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismMotionComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismMotionComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Time = 0.0f;
	MotionQueue.Empty();

	if (Model->Motion != this)
	{
		if (Model->Motion)
		{
			Model->Motion->DestroyComponent();
		}
		Model->Motion = this;
	}
	if (Model && Model->ParameterStore)
	{
		AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	}
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
		Motion->SetFadeout(Motion->FadeOutTime);
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
	if (!Owner || !Owner->Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Owner or Model."));
		return;
	}

	Setup(Owner->Model);

	if (Index >= 0 && Jsons.IsValidIndex(Index))
	{
		PlayMotion(Index, 0.0f, ECubismMotionPriority::Normal);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismMotionComponent: Animation not started (index %d)"), Index);
	}
}

#if WITH_EDITOR
void UCubismMotionComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

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

void UCubismMotionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (!Model) return;

	if (Model->Motion == this)
	{
		Model->Motion = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismMotionComponent::PostEditUndo()
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

void UCubismMotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}
	
	OnCubismUpdate(DeltaTime);
}

int32 UCubismMotionComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_MOTION;
}

void UCubismMotionComponent::UpdateMotion(
	float UserTimeSeconds,
	const TSharedPtr<FCubismMotion>& CubismMotion)
{

	float Elapsed = UserTimeSeconds - CubismMotion->StartTime;
	if (Elapsed < 0.0f)
	{
		Elapsed = 0.0f;
	}

	float MotionTime = Elapsed;
	if (CubismMotion->State == ECubismMotionState::PlayInLoop && CubismMotion->Duration > 0.0f)
	{
		MotionTime = FMath::Fmod(Elapsed, CubismMotion->Duration);
		if (MotionTime < 0.0f)
		{
			MotionTime += CubismMotion->Duration;
		}
	}


	auto EndSecForThisCycle = [&]() -> float
	{
		if (CubismMotion->GetEndTime() >= 0.0f)
		{
			return CubismMotion->GetEndTime();
		}

		if (CubismMotion->State == ECubismMotionState::PlayInLoop && CubismMotion->Duration > 0.0f)
		{
			return CubismMotion->FadeInAnchorTime + CubismMotion->Duration;
		}
		return -1.0f;
	};

	float TailT = -1.0f;
	if (CubismMotion->State == ECubismMotionState::PlayInLoop && CubismMotion->Duration > 0.0f && CubismMotion->Fps > 0.0f)
	{
		const float Delta = 1.0f / CubismMotion->Fps;
		if (MotionTime > CubismMotion->Duration - Delta)
		{
			TailT = (MotionTime - (CubismMotion->Duration - Delta)) / Delta;
			TailT = FMath::Clamp(TailT, 0.0f, 1.0f);
		}
	}

	const float MotionWeight = FMath::Clamp(CubismMotion->GetWeight(), 0.0f, 1.0f);
	const float TmpFadeIn = (CubismMotion->FadeInTime <= 0.0f)
		? 1.0f
		: FCubismMotion::EasingSin((UserTimeSeconds - CubismMotion->FadeInAnchorTime) / CubismMotion->FadeInTime);

	const float EndSecMotion = EndSecForThisCycle();
	const float TmpFadeOut = (CubismMotion->FadeOutTime <= 0.0f || EndSecMotion < 0.0f)
		? 1.0f
		: FCubismMotion::EasingSin((EndSecMotion - UserTimeSeconds) / CubismMotion->FadeOutTime);

	const float MotionFadeWeight = FMath::Clamp(MotionWeight * TmpFadeIn * TmpFadeOut, 0.0f, 1.0f);

	const TArray<FCubismMotionCurve>& Curves = CubismMotion->Curves;

	for (const FCubismMotionCurve& Curve : Curves)
	{
		if (Curve.Target != ECubismMotionCurveTarget::Model) continue;

		float Curr = CubismMotion->GetValue(Curve.Id, MotionTime);
		if (TailT >= 0.0f)
		{
			const float Start = CubismMotion->GetValue(Curve.Id, 0.0f);
			Curr = FMath::Lerp(Curr, Start, TailT);
		}

		if (Curve.Id == "Opacity")
		{
			Model->Opacity = Curr;
		}
	}

	for (const FCubismMotionCurve& Curve : Curves)
	{
		if (Curve.Target != ECubismMotionCurveTarget::Parameter) continue;

		UCubismParameterComponent* Parameter = Model->GetParameter(Curve.Id);
		if (!Parameter) continue;

		const float SourceValue = Parameter->Value;

		float TargetNow = CubismMotion->GetValue(Curve.Id, MotionTime);
		if (Parameter->IsRepeat())
		{
			TargetNow = Parameter->GetParameterRepeatValue(TargetNow);
		}

		if (Curve.FadeInTime < 0.0f && Curve.FadeOutTime < 0.0f)
		{
			const float WeightUsed = MotionFadeWeight;

			float NewValueNow = SourceValue + (TargetNow - SourceValue) * WeightUsed;

			if (TailT >= 0.0f)
			{
				float TargetStart = CubismMotion->GetValue(Curve.Id, 0.0f);
				if (Parameter->IsRepeat())
				{
					TargetStart = Parameter->GetParameterRepeatValue(TargetStart);
				}
				const float NewValueStart = SourceValue + (TargetStart - SourceValue) * WeightUsed;
				NewValueNow = FMath::Lerp(NewValueNow, NewValueStart, TailT);
			}

			Parameter->SetParameterValue(NewValueNow);
			continue;
		}

		float fin = TmpFadeIn;
		float fout = TmpFadeOut;

		if (Curve.FadeInTime >= 0.0f)
		{
			fin = (Curve.FadeInTime == 0.0f)
				? 1.0f
				: FCubismMotion::EasingSin((UserTimeSeconds - CubismMotion->FadeInAnchorTime) / Curve.FadeInTime);
		}

		if (Curve.FadeOutTime >= 0.0f)
		{
			const float EndSecParam = EndSecForThisCycle();
			fout = (Curve.FadeOutTime == 0.0f || EndSecParam < 0.0f)
				? 1.0f
				: FCubismMotion::EasingSin((EndSecParam - UserTimeSeconds) / Curve.FadeOutTime);
		}

		const float ParamWeight = FMath::Clamp(MotionWeight * fin * fout, 0.0f, 1.0f);

		float NewValueNow = SourceValue + (TargetNow - SourceValue) * ParamWeight;

		if (TailT >= 0.0f)
		{
			float TargetStart = CubismMotion->GetValue(Curve.Id, 0.0f);
			if (Parameter->IsRepeat())
			{
				TargetStart = Parameter->GetParameterRepeatValue(TargetStart);
			}
			const float NewValueStart = SourceValue + (TargetStart - SourceValue) * ParamWeight;
			NewValueNow = FMath::Lerp(NewValueNow, NewValueStart, TailT);
		}

		Parameter->SetParameterValue(NewValueNow);
	}

	for (const FCubismMotionCurve& Curve : Curves)
	{
		if (Curve.Target != ECubismMotionCurveTarget::PartOpacity) continue;

		UCubismParameterComponent* Parameter = Model->GetParameter(Curve.Id);
		if (!Parameter) continue;

		float v = CubismMotion->GetValue(Curve.Id, MotionTime);
		if (TailT >= 0.0f)
		{
			const float startV = CubismMotion->GetValue(Curve.Id, 0.0f);
			v = FMath::Lerp(v, startV, TailT);
		}
		Parameter->SetParameterValue(v);
	}
}

void UCubismMotionComponent::OnCubismUpdate(float DeltaTime)
{
	if (!Model)
	{
		return;
	}

	Time += Speed * DeltaTime;

	for (int32 i = 0; i < MotionQueue.Num();)
	{
		TSharedPtr<FCubismMotion>& Motion = MotionQueue[i];

		if (Motion->State == ECubismMotionState::None)
		{
			// Initialize the motion.
			Motion->Init(Time);
			Motion->FadeInAnchorTime = Motion->StartTime;
		}

		float Elapsed = Time - Motion->StartTime;
		if (Elapsed < 0.0f) Elapsed = 0.0f;

		float Phase = Elapsed;
		if (Motion->State == ECubismMotionState::PlayInLoop && Motion->Duration > 0.0f)
		{
			Phase = FMath::Fmod(Elapsed, Motion->Duration);
			if (Phase < 0.0f) Phase += Motion->Duration;
		}

		const bool bLoopSeam =
			(Motion->State == ECubismMotionState::PlayInLoop) &&
			(Motion->Duration > 0.0f) &&
			(Phase + KINDA_SMALL_NUMBER < Motion->PrevPhaseTime);

		if (bLoopSeam && Motion->bLoopFadeIn)
		{
			Motion->FadeInAnchorTime = Time - Phase;
		}

		UpdateMotion(Time, Motion);
		Motion->PrevPhaseTime = Phase;

		if (Motion->IsFinished())
		{
			MotionQueue.RemoveAt(i);
		}
		else
		{
			if (Motion->IsTriggeredFadeOut())
			{
				Motion->StartFadeout(Motion->GetFadeOutSeconds(), Time);
			}

			i++;
		}
	}

	if (IsFinished())
	{
		CurrentPriority = ECubismMotionPriority::None;

		OnMotionPlaybackFinished.Broadcast();
	}
}
// End of UActorComponent interface
