/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Motion/CubismMotion3Json.h"

/**
 * A support class for importing motion3.json files.
 */
class FCubismMotion3JsonImporter
{
public:
	bool ImportFromString(const FString& FileContent);

	void ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismMotion3Json>& Json);

private:
	static TSharedPtr<FJsonObject> ParseJSON(const FString& FileContent, bool bSilent);

	TArray<FRichCurveKey> ParseSegments(const TArray<float>& Segments, int32& NumSegments, int32& NumPoints) const;

protected:
	int32 Version;

	float Duration;

	bool bLoop = false;

	float Fps = 30.0f;

	float FadeInTime = 1.0f;

	float FadeOutTime = 1.0f;

	int32 CurveCount;

	int32 TotalSegmentCount;

	int32 TotalPointCount;

	TArray<FCubismMotionCurve> Curves;

	TMap<FName, TArray<FRichCurveKey>> MotionCurves;

	TArray<FCubismMotionEvent> Events;
};
