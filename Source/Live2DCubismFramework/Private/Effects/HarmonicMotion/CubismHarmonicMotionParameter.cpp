/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/HarmonicMotion/CubismHarmonicMotionParameter.h"

float FCubismHarmonicMotionParameter::CalcValue(const float Time, const float Min, const float Max)
{
	const float Width = Max - Min;

	float Origin = Min + NormalizedOrigin * Width;
	float Range = NormalizedRange * Width;

	switch (Direction)
	{
		case ECubismHarmonicMotionDirection::Left:
		{
			if (Origin - Range >= Min)
			{
				Range *= 0.5f;
				Origin -= Range;
			}
			else
			{
				Range = 0.5f * (Origin - Min);
				Origin = Min + Range;
				NormalizedRange = 2.0f * Range / Width;
			}

			break;
		}
		case ECubismHarmonicMotionDirection::Right:
		{
			if (Origin + Range <= Max)
			{
				Range *= 0.5f;
				Origin += Range;
			}
			else
			{
				Range = 0.5f * (Max - Origin);
				Origin = Max - Range;
				NormalizedRange = 2.0f * Range / Width;
			}

			break;
		}
		default:
		{
			break;
		}
	}

	if (Origin - Range < Min)
	{
		Range = Origin - Min;
		NormalizedRange = Range / Width;
	}
	else if (Origin + Range > Max)
	{
		Range = Max - Origin;
		NormalizedRange = Range / Width;
	}

	return Origin + Range * FMath::Sin(Time * 2.0f * PI / Duration);
}
