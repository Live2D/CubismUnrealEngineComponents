/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Motion/CubismMotion.h"

FCubismMotion::FCubismMotion(const UCubismMotion3Json* Json, const float InOffsetTime)
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

float FCubismMotion::CalcWeight(const FString Id, const float ElapsedTime) const
{
	float FadeInWeight = 1.0f;
	float FadeOutWeight = 1.0f;

	if (FadeInTime > 0.0f)
	{
		FadeInWeight = EasingSin((ElapsedTime - OffsetTime) / FadeInTime);
	}

	if (State != ECubismMotionState::PlayInLoop && FadeOutTime > 0.0f)
	{
		FadeOutWeight = EasingSin((EndTime - ElapsedTime) / FadeOutTime);
	}

	for (const FCubismMotionCurve& Curve : Curves)
	{
		if (Curve.Id != Id)
		{
			continue;
		}

		if (Curve.FadeInTime == 0.0f)
		{
			FadeInWeight = 1.0f;
		}
		else if (Curve.FadeInTime > 0.0f)
		{
			FadeInWeight = EasingSin((ElapsedTime - OffsetTime) / Curve.FadeInTime);
		}

		if (Curve.FadeOutTime == 0.0f || State == ECubismMotionState::PlayInLoop)
		{
			FadeOutWeight = 1.0f;
		}
		else if (Curve.FadeOutTime > 0.0f)
		{
			FadeOutWeight = EasingSin((EndTime - ElapsedTime) / Curve.FadeOutTime);
		}

		break;
	}

	const float FadeWeight = FadeInWeight * FadeOutWeight;

	check(0.0f <= FadeWeight && FadeWeight <= 1.0f);

	return FadeWeight;
}

void FCubismMotion::FadeOut(const float Time)
{
	State = ECubismMotionState::Play;

	const float NewEndTime = FadeOutTime + Time - StartTime;

	if (NewEndTime < EndTime)
	{
		EndTime = NewEndTime;
	}
}
