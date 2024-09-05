/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Rendering/CubismMaskTextureComponent.h"

class UCubismDrawableComponent;
class UTextureRenderTarget2D;

/**
 * A class that manages the address of the mask to be drawn.
 */
class FCubismMaskJunction
{
public:
	/**
	 * The list of the drawables that use the same mask.
	 */
	TArray<TObjectPtr<UCubismDrawableComponent>> Drawables;

	/**
	 * The list of the drawables for masking.
	 */
	TArray<TObjectPtr<UCubismDrawableComponent>> MaskDrawables;

	/**
	 * The render target where the mask is drawn.
	 */
	UTextureRenderTarget2D* RenderTarget;

	/**
	 * The offset of the mask to be drawn.
	 */
	FVector4 Offset;

	/**
	 * The channel of the mask to be drawn.
	 */
	FVector4 Channel;
};
