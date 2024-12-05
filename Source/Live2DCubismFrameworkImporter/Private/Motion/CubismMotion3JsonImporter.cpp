/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Motion/CubismMotion3JsonImporter.h"

#include "AssetToolsModule.h"
#include "Json.h"
#include "CubismLog.h"

#define ASSERT(condition, message) if (!ensure(condition)) { UE_LOG(LogCubism, Error, TEXT("%s"), TEXT(message)); return false; }

TSharedPtr<FJsonObject> FCubismMotion3JsonImporter::ParseJSON(const FString& FileContent, bool bSilent)
{
	const TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(FileContent);
	TSharedPtr<FJsonObject> JsonObject;

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		return JsonObject;
	}
	else
	{
		if (!bSilent)
		{
			UE_LOG(LogCubism, Error, TEXT("Failed to deserialize JSON object.  Error: '%s'"), *JsonReader->GetErrorMessage());
		}
		return nullptr;
	}
}

bool FCubismMotion3JsonImporter::ImportFromString(const FString& FileContent)
{
	const TSharedPtr<FJsonObject>& JsonObject = ParseJSON(FileContent, false);

	if (!JsonObject.IsValid())
	{
		return false;
	}

	ASSERT(JsonObject->TryGetNumberField(TEXT("Version"), Version), "Failed to get Version field.");

	const TSharedPtr<FJsonObject>* Meta;
	ASSERT(JsonObject->TryGetObjectField(TEXT("Meta"), Meta), "Failed to get Meta field.");
	{
		ASSERT((*Meta)->TryGetNumberField(TEXT("Duration"), Duration), "Failed to get Duration field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("Fps"), Fps), "Failed to get Fps field.");
		(*Meta)->TryGetNumberField(TEXT("FadeInTime"), FadeInTime);
		(*Meta)->TryGetNumberField(TEXT("FadeOutTime"), FadeOutTime);
		ASSERT((*Meta)->TryGetBoolField(TEXT("Loop"), bLoop), "Failed to get Loop field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("CurveCount"), CurveCount), "Failed to get CurveCount field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("TotalSegmentCount"), TotalSegmentCount), "Failed to get TotalSegmentCount field.");
		ASSERT((*Meta)->TryGetNumberField(TEXT("TotalPointCount"), TotalPointCount), "Failed to get TotalPointCount field.");
	}

	const TArray< TSharedPtr<FJsonValue> >*  CurvesArrayObject;
	ASSERT(JsonObject->TryGetArrayField(TEXT("Curves"), CurvesArrayObject), "Failed to get Curves field.");
	{
		int32 NumSegments = 0;
		int32 NumPoints = 0;

		for (const TSharedPtr<FJsonValue>& CurveValue : *CurvesArrayObject)
		{
			FCubismMotionCurve Curve;

			const TSharedPtr<FJsonObject>& CurveObject = CurveValue->AsObject();

			FString Target;
			ASSERT(CurveObject->TryGetStringField(TEXT("Target"), Target), "Failed to get Target field.");
			if (Target == TEXT("Model"))
			{
				Curve.Target = ECubismMotionCurveTarget::Model;
			}
			else if (Target == TEXT("Parameter"))
			{
				Curve.Target = ECubismMotionCurveTarget::Parameter;
			}
			else if (Target == TEXT("PartOpacity"))
			{
				Curve.Target = ECubismMotionCurveTarget::PartOpacity;
			}
			else
			{
				Curve.Target = ECubismMotionCurveTarget::None;
				ensure(false);
			}
			ASSERT(CurveObject->TryGetStringField(TEXT("Id"), Curve.Id), "Failed to get Id field.");
			CurveObject->TryGetNumberField(TEXT("FadeInTime"), Curve.FadeInTime);
			CurveObject->TryGetNumberField(TEXT("FadeOutTime"), Curve.FadeOutTime);

			Curves.Add(Curve);

			const TArray<TSharedPtr<FJsonValue>>* SegmentsArrayObject;
			ASSERT(CurveObject->TryGetArrayField(TEXT("Segments"), SegmentsArrayObject), "Failed to get Segments field.");
			{
				TArray<float> Segments;

				for (const TSharedPtr<FJsonValue>& SegmentValue : *SegmentsArrayObject)
				{
					Segments.Add(SegmentValue->AsNumber());
				}

				MotionCurves.Add(*Curve.Id, ParseSegments(Segments, NumSegments, NumPoints));
			}
		}

		if (Curves.Num() != CurveCount)
		{
			UE_LOG(LogCubism, Error, TEXT("Curves count mismatch. Expected: %d, Actual: %d"), CurveCount, Curves.Num());
			return false;
		}

		if (NumSegments != TotalSegmentCount)
		{
			UE_LOG(LogCubism, Error, TEXT("Segments count mismatch. Expected: %d, Actual: %d"), TotalSegmentCount, NumSegments);
			return false;
		}

		if (NumPoints != TotalPointCount)
		{
			UE_LOG(LogCubism, Error, TEXT("Points count mismatch. Expected: %d, Actual: %d"), TotalPointCount, NumPoints);
			return false;
		}
	}

	return true;
}

void FCubismMotion3JsonImporter::ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismMotion3Json>& Json)
{
	Json->Duration = Duration;
	Json->bLoop = bLoop;
	Json->Fps = Fps;
	Json->FadeInTime = FadeInTime;
	Json->FadeOutTime = FadeOutTime;
	Json->Curves.Append(Curves);
	
	UCurveTable* CurveTable = NewObject<UCurveTable>(Json, TEXT("CurveTable"), Flags);
	
	for (const auto& Entry : MotionCurves)
	{
		FRichCurve& Curve = CurveTable->AddRichCurve(Entry.Key);
		Curve.SetKeys(Entry.Value);
	}

	Json->CurveTable = CurveTable;
}

TArray<FRichCurveKey> FCubismMotion3JsonImporter::ParseSegments(const TArray<float> &Segments, int32& NumSegments, int32& NumPoints) const
{
	TArray<FRichCurveKey> KeyFrames;

	KeyFrames.Add(FRichCurveKey(Segments[0], Segments[1]));
	NumPoints += 1;

	int32 i = 2;
	while (i < Segments.Num())
	{
		FRichCurveKey K0 = KeyFrames.Pop();

		const int32 CurveType = Segments[i];
		switch (CurveType)
		{
			case 0: // Linear
			{
				const float t1 = Segments[i + 1];
				const float v1 = Segments[i + 2];

				FRichCurveKey K1(t1, v1);

				K0.InterpMode = RCIM_Linear;

				KeyFrames.Add(K0);
				KeyFrames.Add(K1);

				i += 3;
				NumPoints += 1;

				break;
			}
			case 1: // 3rd order Bezier
			{
				const float t0 = K0.Time;
				const float v0 = K0.Value;
				const float ta = Segments[i + 1];
				const float va = Segments[i + 2];
				const float tb = Segments[i + 3];
				const float vb = Segments[i + 4];
				const float t1 = Segments[i + 5];
				const float v1 = Segments[i + 6];

				FRichCurveKey K1(t1, v1);

				const float OneThird = 1.0f / 3.0f;

				K0.InterpMode = RCIM_Cubic;
				K0.LeaveTangent = (va - v0) / (ta - t0);
				K0.LeaveTangentWeight = (ta - t0) * FMath::Sqrt(1.0f + K0.LeaveTangent * K0.LeaveTangent);
				K0.TangentWeightMode = K0.ArriveTangentWeight? (K0.LeaveTangentWeight? RCTWM_WeightedBoth : RCTWM_WeightedArrive) : (K0.LeaveTangentWeight? RCTWM_WeightedLeave : RCTWM_WeightedNone);
				K0.TangentMode = RCTM_Break;

				K1.ArriveTangent = (v1 - vb) / (t1 - tb);
				K1.ArriveTangentWeight = (t1 - tb) * FMath::Sqrt(1.0f + K1.ArriveTangent * K1.ArriveTangent);
				K1.TangentWeightMode = RCTWM_WeightedArrive;
				K1.TangentMode = RCTM_Break;

				KeyFrames.Add(K0);
				KeyFrames.Add(K1);

				i += 7;
				NumPoints += 3;

				break;
			}
			case 2: // Stepped
			{
				const float t1 = Segments[i + 1];
				const float v1 = Segments[i + 2];

				FRichCurveKey K1(t1, v1);

				K0.InterpMode = RCIM_Constant;

				KeyFrames.Add(K0);
				KeyFrames.Add(K1);

				i += 3;
				NumPoints += 1;

				break;
			}
			case 3: // InverseStepped
			{
				const float t1 = K0.Time + 1.0e-6;
				const float v1 = Segments[i + 2];
				const float t2 = Segments[i + 1];
				const float v2 = Segments[i + 2];

				FRichCurveKey K1(t1, v1), K2(t2, v2);

				K0.InterpMode = RCIM_Linear;
				K1.InterpMode = RCIM_Constant;

				KeyFrames.Add(K0);
				KeyFrames.Add(K1);
				KeyFrames.Add(K2);

				i += 3;
				NumPoints += 1;

				break;
			}
			default:
			{
				ensure(false);

				break;
			}
		}

		NumSegments += 1;
	}

	return KeyFrames;
}
