/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"
#include "Engine/CurveTable.h"
#include "CubismMotion3Json.generated.h"

/**
 * An enumeration to specify the target of motion.
 */
UENUM(BlueprintType)
enum class ECubismMotionCurveTarget : uint8
{
	None,
	Model,
	Parameter,
	PartOpacity,
};

/**
 * A structure corresponding to an array element of the `Curves` field in a motion object.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismMotionCurve
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a curve object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Curve")
	FString Id;

	/**
	 * The property corresponding to the `Target` field in a curve object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Curve")
	ECubismMotionCurveTarget Target;

	/**
	 * The property corresponding to the `FadeInTime` field in a curve object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Curve")
	float FadeInTime;

	/**
	 * The property corresponding to the `FadeOutTime` field in a curve object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Curve")
	float FadeOutTime;

	FCubismMotionCurve()
		: Id()
		, Target(ECubismMotionCurveTarget::None)
		, FadeInTime(-1.0f)
		, FadeOutTime(-1.0f)
	{ }
};

/**
 * A structure corresponding to an array element of the `Events` field in a motion object.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismMotionEvent
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Time` field in an event object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	float Time;

	/**
	 * The property corresponding to the `Value` field in an event object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	FString Value;

	FCubismMotionEvent()
		: Time(0.0f)
		, Value()
	{ }
};

/**
 * A class to hold the motion data read from motion3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismMotion3Json : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The property corresponding to the `Duration` field in a `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	float Duration;

	/**
	 * The property corresponding to the `Loop` field in a `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	bool bLoop;

	/**
	 * The property corresponding to the `Fps` field in a `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	float Fps;

	/**
	 * The property corresponding to the `FadeInTime` field in a `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	float FadeInTime;

	/**
	 * The property corresponding to the `FadeOutTime` field in a `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	float FadeOutTime;

	/**
	 * The property corresponding to the `Curves` field in a motion object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	TArray<FCubismMotionCurve> Curves;

	/**
	 * The property corresponding to the `Curves` field in a motion object.
	 * The object specifying the list of motion curves determined from `Id` and `Segments` fields.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	TObjectPtr<UCurveTable> CurveTable;

	/**
	 * The property corresponding to the `Events` field in a motion object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
	TArray<FCubismMotionEvent> Events;

	// UObject interface
	virtual void PostInitProperties() override;
	// End of UObject interface

#if WITH_EDITORONLY_DATA
	// Import data for this 
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSettings)
	TObjectPtr<class UAssetImportData> AssetImportData;

	// UObject interface
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface
#endif
};

/**
 * A structure to hold the group of motion data.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FMotion3JsonGroup
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Name` field in a motion group object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Group")
	FString Name;

	/**
	 * The list of motion data.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Group")
	TArray<TObjectPtr<UCubismMotion3Json>> Motion3Jsons;

	FMotion3JsonGroup()
		: Name()
		, Motion3Jsons()
	{ }
};
