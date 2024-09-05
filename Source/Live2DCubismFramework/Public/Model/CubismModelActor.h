/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismModelActor.generated.h"

/**
 * An instance of a Live2D Cubism model in a level.
 * This actor is created when you drag a model asset(UCubismModel3Json) from the content browser into the level, and
 * it is just a thin wrapper around a UCubismModelComponent that actually references the asset.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API ACubismModel : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * A component to control the model.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<class UCubismModelComponent> Model;

};
