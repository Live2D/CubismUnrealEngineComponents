/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"

#include "CubismLookAtParameter.generated.h"

/**
 * An enumeration to specify the dimension of the look-at parameter to project.
 */
UENUM(BlueprintType)
enum class ECubismLookAtAxis : uint8
{
	X,
	Y,
	Z,
};

/**
 * A structure to store the parameters to control the look-at effect.
*/
USTRUCT(BlueprintType)
struct FCubismLookAtParameter
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * The value of the look-at parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	float Value = 0.0f;

	/**
	 * The mode to blend the look-at parameter.
	 * The default is Additive.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismParameterBlendMode BlendMode = ECubismParameterBlendMode::Additive;

	/**
	 * The flag to enable the look-at effect.
	 * The default is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bEnabled = false;

	/**
	 * The axis to project the look-at parameter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bEnabled"), Category = "Live2D Cubism")
	ECubismLookAtAxis Axis = ECubismLookAtAxis::X;

	/**
	 * The factor to multiply the look-at parameter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"), meta = (ClampMin = "-100.0", SliderMin = "0.0", SliderMax = "100.0"), Category = "Live2D Cubism")
	float Factor = 1.0f;

	/**
	 * The ID of the UCubismParameterComponent to which the look-at parameter is applied.
	 */
	UPROPERTY(EditAnywhere, Category = "Live2D Cubism")
	FString Id = TEXT("");
};
