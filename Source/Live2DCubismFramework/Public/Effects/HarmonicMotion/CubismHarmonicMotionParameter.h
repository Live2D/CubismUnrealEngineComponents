/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"

#include "CubismHarmonicMotionParameter.generated.h"

UENUM(BlueprintType)
enum class ECubismHarmonicMotionDirection : uint8
{
	Centric,
	Left,
	Right,
};

/**
 * A structure to store the parameters to control the harmonic motion.
 */
USTRUCT(BlueprintType)
struct FCubismHarmonicMotionParameter
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * The value of the harmonic motion.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	float Value = 0.0f;

	/**
	 * The mode to blend the harmonic motion parameter.
	 * The default is Additive.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismParameterBlendMode BlendMode = ECubismParameterBlendMode::Additive;

	/**
	 * The flag to enable the harmonic motion.
	 * The default is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bEnabled = false;

	/**
	 * The time scale of the harmonic motion.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", SliderMin="0.0", SliderMax="100.0"), Category = "Live2D Cubism")
	float TimeScale = 1.0f;

	/**
	 * The direction of the harmonic motion.
	 */
	UPROPERTY(EditAnywhere, Category = "Live2D Cubism")
	ECubismHarmonicMotionDirection Direction = ECubismHarmonicMotionDirection::Centric;

	/**
	 * The normalized origin of the harmonic motion.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0", SliderMin="0.0", SliderMax="1.0"), Category = "Live2D Cubism")
	float NormalizedOrigin = 0.5f;

	/**
	 * The normalized range of the harmonic motion.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0", SliderMin="0.0", SliderMax="1.0"), Category = "Live2D Cubism")
	float NormalizedRange = 0.5f;

	/**
	 * The duration of the harmonic motion.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.01", ClampMax="10.0", SliderMin="0.01", SliderMax="10.0"), Category = "Live2D Cubism")
	float Duration = 3.0f;

	/**
	 * The ID of the UCubismParameterComponent to which the parameter is applied.
	 */
	UPROPERTY(EditAnywhere, Category = "Live2D Cubism")
	FString Id = TEXT("");

public:
	float CalcValue(const float Time, const float Min, const float Max);
};
