/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Motion/CubismMotion3Json.h"
#include "Engine/CurveTable.h"

/**
 * An enumeration of the states of the motion.
 */
enum class ECubismMotionState : uint8
{
	None,
	Play,
	PlayInLoop,
	End,
};

/**
 * A class to control a state of the motion in the queue.
 */
class FCubismMotion
{
public:

	/**
	 * The constructor.
	 * @param Json The json asset that contains the motion information.
	 */
	FCubismMotion(const UCubismMotion3Json* Json, const float InOffsetTime);

	/**
	 * The duration of the motion. (seconds)
	 */
	float Duration;

	/**
	 * The flag to determine whether to loop the motion.
	 */
	bool bLoop;

	/**
	 * The frame rate of the motion.
	 */
	float Fps;

	/**
	 * The time to fade in the motion. (seconds)
	 */
	float FadeInTime;

	/**
	 * The time to fade out the motion. (seconds)
	 */
	float FadeOutTime;

	/**
	 * The curves of the motion.
	 */
	TArray<FCubismMotionCurve> Curves;

	/**
	 * The curve table that contains the curves.
	 */
	TObjectPtr<UCurveTable> CurveTable;

	/**
	 * The events of the motion.
	 */
	TArray<FCubismMotionEvent> Events;

	/**
	 * The start time of the motion. (seconds)
	 */
	float StartTime;

	/**
	 * The delay time from the start of the motion to the start of the fade-in. (seconds)
	 */
	float OffsetTime;

	/**
	 * The end time of the motion. (seconds)
	 */
	float EndTime;

public:
	/**
	 * The function to initialize the state of the motion.
	 * @param Time The current time.
	 */
	void Init(const float Time);

	/**
	 * The function to calculate the weight of the fade in/out.
	 * @param Time The current time.
	 * @return The weight of the fade in and the fade in/out.
	 */
	float UpdateFadeWeight(const TSharedPtr<FCubismMotion>& CubismMotion, float UserTimeSeconds);

	/**
	 * The function to fade out the motion.
	 * @param Time The start time of the fade out.
	 */
	void FadeOut(const float Time);

	/**
	 * The function to set weight value.
	 *  @param The weight value of the motion.
	 */
	void SetWeight(float weight);

	/**
	 * The function to get weight value.
	 * @return The weight value of the motion.
	 */
	float GetWeight() const;

	/**
	 * Set the time when fade out ends.
	 * @param The time at which the fade ends.
	 */
	void SetFadeout(float NewFadeOutSeconds);

	/**
	 * Initiate fade out.
	 * @param The time at which the fade ends.
	 * @param Time The current time.
	 */
	void StartFadeout(float NewFadeOutSeconds, float UserTimeSeconds);

	/**
	 * Whether fade out has started.
	 * @return True if the fade out flag is set.
	 */
	bool IsTriggeredFadeOut();

	/**
	 * Get fade out time.
	 * @return The fade out time.
	 */
	float GetFadeOutSeconds();

	/**
	 * End time of motion playback.
	 * @return Time at which motion playback ends.
	 */
	float GetEndTime();

	/**
	 * Whether the motion has finished playing or not.
	 * @param True if the motion has finished playing.
	 */
	void IsFinished(bool F);

	/**
	 * Whether the motion has finished playing or not.
	 * @return True if the motion has finished playing.
	 */
	bool IsFinished() const;

private:
	friend class UCubismMotionComponent;

	/**
	 * The state of the motion.
	 */
	ECubismMotionState State = ECubismMotionState::None;

	static inline float EasingSin(const float Value)
	{
		if (Value < 0.0f)
		{
			return 0.0f;
		}

		if (Value >= 1.0f)
		{
			return 1.0f;
		}

		return 0.5f - 0.5f * FMath::Cos(PI * Value);
	}

	float GetValue(const FString Id, const float Time) const
	{
		return CurveTable->FindRichCurve(*Id, Id)->Eval(Time, 0.0f);
	}

	float Weight;

	float FadeOutSeconds;

	float EndTimeSeconds;

	bool bIsTriggeredFadeOut;

	bool bFinished;
};
