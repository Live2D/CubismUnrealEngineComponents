/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Rendering/CubismMaskTextureComponent.h"

#include "CubismMaskTexture.generated.h"

class UCubismMaskTextureComponent;

/**
 * An instance of a Live2D Cubism mask texture in a level.
 * This actor is created automatically when an ACubismModel is spawned in the level.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API ACubismMaskTexture : public AActor
{
	GENERATED_BODY()

private:
	/**
	 * @brief The constructor of the actor.
	 */
	ACubismMaskTexture()
	{
		MaskTextureComponent = CreateDefaultSubobject<UCubismMaskTextureComponent>(TEXT("CubismTextureComponent"));

		#if WITH_EDITORONLY_DATA
			bIsSpatiallyLoaded = false;
		#endif
	}

private:
	friend class UCubismRendererComponent;

	/**
	 * A component to control the mask texture.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintReadOnly, meta=(ExposeFunctionCategories="Sprite,Rendering,Physics,Components", AllowPrivateAccess="true"), Category = "Live2D Cubism")
	TObjectPtr<UCubismMaskTextureComponent> MaskTextureComponent;
};
