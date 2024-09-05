/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "BatchedElements.h"

class FCubismMaskBatchedElementParameters : public FBatchedElementParameters
{
public:
	FCubismMaskBatchedElementParameters(
		const FVector4 InOffset,
		const FVector4 InChannel,
		const FTextureResource* InMainTexture
	)
		: Offset(InOffset)
		, Channel(InChannel)
		, MainTexture(InMainTexture)
	{ }

	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

protected:
	/** The offset of the mask to be drawn. */
	const FVector4 Offset;

	/** The channel of the mask to be drawn. */
	const FVector4 Channel;

	/** The texture assigned to the drawable. */
	const FTextureResource* MainTexture;
};
