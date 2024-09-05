/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Expression/CubismExpression.h"

FCubismExpression::FCubismExpression(const UCubismExp3Json* Json)
{
	Weight = 1.0f;
	FadeInTime = Json->FadeInTime;
	FadeOutTime = Json->FadeOutTime;
	Parameters = Json->Parameters;	
}

void FCubismExpression::Init(const float Time)
{
	State = ECubismExpressionState::Play;

	StartTime = Time;
	EndTime = INFINITY;
}

float FCubismExpression::CalcExpressionWeight(const float Time) const
{
	float CalcWeight = 1.0f;

	if (FadeInTime > 0.0f)
	{
		CalcWeight = EasingSin((Time - StartTime) / FadeInTime);
	}

	check(CalcWeight >= 0.0f && CalcWeight <= 1.0f);

	return CalcWeight;
}

float FCubismExpression::UpdateWeight(const float ElapsedTime)
{
	float FadeInWeight = 1.0f;
	float FadeOutWeight = 1.0f;
	float NewFadeWeight = Weight;

	if (FadeInTime > 0.0f)
	{
		FadeInWeight = EasingSin(ElapsedTime / FadeInTime);
	}

	if (FadeOutTime > 0.0f)
	{
		FadeOutWeight = EasingSin((EndTime - ElapsedTime) / FadeOutTime);
	}

	NewFadeWeight = NewFadeWeight * FadeInWeight * FadeOutWeight;

	check(NewFadeWeight >= 0.0f && NewFadeWeight <= 1.0f);

	return NewFadeWeight;
}

void FCubismExpression::StartFadeout(const float Time)
{
	const float NewEndTime = FadeOutTime + Time;

	if (EndTime < 0.0f || NewEndTime < EndTime)
	{
		EndTime = NewEndTime;
	}
}
