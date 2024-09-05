/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismRaycastParameter.generated.h"

/**
 * An enumeration of the precision of the raycast.
 */
UENUM(BlueprintType)
enum class ECubismRaycastPrecision : uint8
{
	BoundingBox,
	Mesh
};

/**
 * A structure to hold the raycast parameter.
 */
USTRUCT(BlueprintType)
struct FCubismRaycastParameter
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * The flag to enable the parameter as the target of the raycast.
	 * The default value is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bEnabled = false;

	/**
	 * The precision of the raycast.
	 * The default value is BoundingBox.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismRaycastPrecision Precision = ECubismRaycastPrecision::BoundingBox;

	/**
	 * The ID of the UCubismDrawableComponent to be the target of the raycast.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FString Id = TEXT("");

	/**
	 * The name of the UCubismDrawableComponent to be the target of the raycast.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	FString Name = TEXT("");
};
