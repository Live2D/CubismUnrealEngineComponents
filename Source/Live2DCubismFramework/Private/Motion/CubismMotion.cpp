/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Motion/CubismMotion.h"

FCubismMotion::FCubismMotion(const UCubismMotion3Json* Json, const float InOffsetTime)
	: Weight(1.0f)
	, FadeOutSeconds(0.0f)
	, EndTimeSeconds(-1.0f)
	, bIsTriggeredFadeOut(false)
	, bFinished(false)
{
	Duration = Json->Duration;
	bLoop = Json->bLoop;
	Fps = Json->Fps;
	FadeInTime = Json->FadeInTime;
	FadeOutTime = Json->FadeOutTime;
	Curves = Json->Curves;
	CurveTable = Json->CurveTable;
	Events = Json->Events;

	OffsetTime = InOffsetTime;
}

void FCubismMotion::Init(const float Time)
{
	State = (bLoop? ECubismMotionState::PlayInLoop : ECubismMotionState::Play);

	StartTime = Time;
	EndTime = Duration;
}

float FCubismMotion::UpdateFadeWeight(const TSharedPtr<FCubismMotion>& CubismMotion, float UserTimeSeconds)
{
	float FadeWeight = Weight;

	const float FadeIn = CubismMotion->FadeInTime == 0.0f
		? 1.0f
		: EasingSin((UserTimeSeconds - CubismMotion->StartTime) / CubismMotion->FadeInTime);

	const float FadeOut = (CubismMotion->FadeOutTime == 0.0f || CubismMotion->GetEndTime() < 0.0f)
		? 1.0f
		: EasingSin((CubismMotion->GetEndTime() - UserTimeSeconds) / CubismMotion->FadeOutTime);

	FadeWeight = FadeWeight * FadeIn * FadeOut;

	check(0.0f <= FadeWeight && FadeWeight <= 1.0f);

	return FadeWeight;
}

void FCubismMotion::SetWeight(float MotionWeight)
{
	this->Weight = MotionWeight;
}

float FCubismMotion::GetWeight() const
{
	return Weight;
}
void FCubismMotion::SetFadeout(float NewFadeOutSeconds)
{
	this->FadeOutSeconds = NewFadeOutSeconds;
	bIsTriggeredFadeOut = true;
}

void FCubismMotion::FadeOut(const float Time)
{
	State = ECubismMotionState::Play;

	const float NewEndTime = FadeOutTime + Time;

	if (NewEndTime < EndTime)
	{
		EndTime = NewEndTime;
	}
}

void FCubismMotion::StartFadeout(float NewFadeOutSeconds, float UserTimeSeconds)
{
	const float NewEndTimeSeconds = UserTimeSeconds + NewFadeOutSeconds;
	bIsTriggeredFadeOut = true;

	if (EndTimeSeconds < 0.0f || NewEndTimeSeconds < EndTimeSeconds)
	{
		EndTimeSeconds = NewEndTimeSeconds;
	}
}

bool FCubismMotion::IsTriggeredFadeOut()
{
	return bIsTriggeredFadeOut;
}

float FCubismMotion::GetFadeOutSeconds()
{
	return FadeOutSeconds;
}

float FCubismMotion::GetEndTime()
{
	return EndTimeSeconds;
}

void FCubismMotion::IsFinished(bool F)
{
	bFinished = F;
}

bool FCubismMotion::IsFinished() const
{
	return bFinished;
}
