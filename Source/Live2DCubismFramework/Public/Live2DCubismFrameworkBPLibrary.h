/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Model/CubismModel3Json.h"
#include "Model/CubismModelActor.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Live2DCubismFrameworkBPLibrary.generated.h"

UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API ULive2DCubismFrameworkBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Load the model into the spawned actor and build all necessary Cubism components
	 * 
	 * @param WorldContextObject The context object for the world
	 * @param Model3Json The model asset to load
	 * @param Transform The transform to set for the spawned actor
	 * @param bRenderInWorldSpace Whether to render the model in world space or not
	 * @param RenderTarget The render target to draw the model to (optional)
	 * @return The spawned model actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism", meta = (WorldContext = "WorldContextObject"))
	static ACubismModel* SpawnCubismModel(
		UObject* WorldContextObject,
		UCubismModel3Json* Model3Json,
		const FTransform& Transform,
		const bool bRenderInWorldSpace = true,
		UTextureRenderTarget2D* RenderTarget = nullptr
	);
};
