/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "TextureResource.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Runtime/Launch/Resources/Version.h"

class FCubismMaskRenderer;
class UCubismDrawableComponent;

/**
 * Handles the rendering of masks for models.
 * This class manages the resources and rendering logic necessary to draw mask textures based on the drawable components of the model.
 */
class FCubismMaskRenderer final
{
public:
	struct FMeshBuffer
	{
		FBufferRHIRef VertexBuffer;
		FBufferRHIRef IndexBuffer;
	};

	struct FMeshVertexData
	{
		FVector2f Position; // ATTRIBUTE0
		FVector2f UV;       // ATTRIBUTE1
	};

	struct FDrawableInfo
	{
		TArray<uint16> Indices;
		TArray<FMeshVertexData> Vertices;
		FVector4 Offset;
		FVector4 Channel;
		FTexture* MainTexture;
		FCubismMaskRenderer* Renderer;
	};


	explicit FCubismMaskRenderer(int64 InNumVertices, int64 InNumIndices);
	
	~FCubismMaskRenderer();


	/// Draws the mask textures for the given drawable components onto the specified render target resource.
	static void DrawMeshes_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* MaskRenderTarget, const TArray<FDrawableInfo>& MaskDrawableInfoArray, ERHIFeatureLevel::Type FeatureLevel);

private:
	void InitResource_RenderThread(FRHICommandListBase& RHICmdList);
	void ReleaseResource();

	void DrawMesh_RenderThread(FRHICommandList& RHICmdList, const FDrawableInfo& MaskDrawableInfo);


	FMeshBuffer MaskMeshBuffer;
	int64 NumVertices;
	int64 NumIndices;

	bool bIsInitialized;
};

