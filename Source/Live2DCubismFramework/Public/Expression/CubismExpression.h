/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Expression/CubismExp3Json.h"

/**
 * An enumeration of the states of the expression.
 */
enum class ECubismExpressionState : uint8
{
	None,
	Play,
	End,
};

/**
 * A class to control a state of the expression in the queue.
 */
class FCubismExpression
{
public:

	/**
	 * The constructor.
	 * @param Json The json asset that contains the expression information.
	 */
	FCubismExpression(const UCubismExp3Json* Json);

	/**
	 * The time to fade in the expression. (seconds)
	 */
	float FadeInTime;

	/**
	 * The time to fade out the expression. (seconds)
	 */
	float FadeOutTime;

	/**
	 * The weight of the expression.
	 */
	float Weight;

	/**
	 * The parameters of the expression.
	 */
	TArray<FCubismExpressionParameter> Parameters;

	/**
	 * The start time of the expression. (seconds)
	 */
	float StartTime;

	/**
	 * The end time of the expression. (seconds)
	 */
	float EndTime;

public:
	/**
	 * The function to initialize the state of the expression.
	 * @param Time The current time.
	 */
	void Init(const float Time);

	/**
	 * The function to calculate the weight of the fade in.
	 * @param Time The current time.
	 * @return The weight of the fade in.
	 */
	float CalcExpressionWeight(const float Time) const;

	/**
	 * The function to calculate the weight of the fade in/out.
	 * @param Time The current time.
	 * @return The weight of the fade in and the fade in/out.
	 */
	float UpdateWeight(const float ElapsedTime);

	/**
	 * The function to fade out the expression.
	 * @param Time The start time of the fade out.
	 */
	void StartFadeout(const float Time);

private:
	friend class UCubismExpressionComponent;

	/**
	 * The state of the expression.
	 */
	ECubismExpressionState State = ECubismExpressionState::None;

	/**
	 * The weight of the fade in/out.
	 */
	float FadeWeight = 0.0f;

	static inline float EasingSin(const float Value)
	{
		return Value < 0.0f? 0.0f : Value < 1.0f? 0.5f - 0.5f * FMath::Cos(PI * Value) : 1.0f;
	}
};
