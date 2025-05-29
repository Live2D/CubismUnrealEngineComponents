/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismShaders.h"

#include "ClearQuad.h"
#include "PipelineStateCache.h"
#include "RenderGraphResources.h"
#include "RHIStaticStates.h"

class FCubismMeshVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FCubismMeshVertexDeclaration() {}

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	#else
	virtual void InitRHI() override
	#endif
	{
		FVertexDeclarationElementList Elements;
		uint16 Stride = sizeof(FCubismMeshVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismMeshVertex, Position), VET_Float2, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismMeshVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FCubismMeshVertexDeclaration> GCubismMeshVertexDeclaration;

void DrawCubismMesh_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, const TArray<FDrawInfo>& DrawInfos)
{
	FRHIRenderPassInfo RPInfo(RenderTargetResource->GetRenderTargetTexture(), ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawCubismMesh"));

	// Clear the render target.
	DrawClearQuad(RHICmdList, FLinearColor::Transparent);

	FGraphicsPipelineStateInitializer GraphicsPSOInit;

	TShaderMapRef<FCubismMeshVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	TShaderMapRef<FCubismMeshPS<ECubismMeshPSType::Normal>> PixelShaderNormal(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FCubismMeshPS<ECubismMeshPSType::Masked>> PixelShaderMasked(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FCubismMeshPS<ECubismMeshPSType::InvertedMasked>> PixelShaderInvertedMasked(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismMeshVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;

	for (const FDrawInfo& DrawInfo : DrawInfos)
	{
		int32 NumVerts = DrawInfo.Vertices.Num();
		int32 NumIndices = DrawInfo.Indices.Num();

		if (NumVerts == 0 || NumIndices == 0)
		{
			continue;
		}

		switch (DrawInfo.BlendMode)
		{
			case ECubismDrawableBlendMode::Normal:
				GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						// # pre-multiplied alpha
						// C_out = (C_src * A_src) * 1 + C_dst * (1 - A_src)
						//       = C_src * A_src + C_dst * (1 - A_src)
						BO_Add, BF_One, BF_InverseSourceAlpha,
						// A_out = A_src * 1 + A_dst * (1 - A_src)
						//       = A_src + A_dst * (1 - A_src)
						BO_Add, BF_One, BF_InverseSourceAlpha
					>::GetRHI();
				break;
			case ECubismDrawableBlendMode::Additive:
				GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						// # pre-multiplied alpha
						// C_out = (C_src * A_src) * 1 + C_dst * 1
						//       = C_src * A_src + C_dst
						BO_Add, BF_One, BF_One,
						// A_out = A_src * 0 + A_dst * 1
						//       = A_dst
						BO_Add, BF_Zero, BF_One
					>::GetRHI();
				break;
			case ECubismDrawableBlendMode::Multiplicative:
				GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						// # pre-multiplied alpha
						// C_out = (C_src * A_src) * C_dst + C_dst * (1 - A_src)
						//       = C_src * C_dst * A_src + C_dst * (1 - A_src)
						BO_Add, BF_DestColor, BF_InverseSourceAlpha,
						// A_out = A_src * 0 + A_dst * 1
						//       = A_dst
						BO_Add, BF_Zero, BF_One
					>::GetRHI();
				break;
		}

		if (DrawInfo.IsMasked)
		{
			if (DrawInfo.InvertedMask)
			{
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShaderInvertedMasked.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

				FCubismMeshPS<ECubismMeshPSType::InvertedMasked>::FParameters ParametersPS;

				ParametersPS.MainTexture = DrawInfo.MainTexture->TextureRHI;
				ParametersPS.MainSampler = TStaticSamplerState<>::GetRHI();
				ParametersPS.BaseColor = DrawInfo.BaseColor;
				ParametersPS.MultiplyColor = DrawInfo.MultiplyColor;
				ParametersPS.ScreenColor = DrawInfo.ScreenColor;
				ParametersPS.MaskTexture = DrawInfo.MaskTexture->TextureRHI;
				ParametersPS.MaskSampler = TStaticSamplerState<>::GetRHI();
				ParametersPS.Offset = DrawInfo.Offset;
				ParametersPS.Channel = DrawInfo.Channel;
		
				SetShaderParameters(RHICmdList, PixelShaderInvertedMasked, PixelShaderInvertedMasked.GetPixelShader(), ParametersPS);
			}
			else
			{
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShaderMasked.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

				FCubismMeshPS<ECubismMeshPSType::Masked>::FParameters ParametersPS;

				ParametersPS.MainTexture = DrawInfo.MainTexture->TextureRHI;
				ParametersPS.MainSampler = TStaticSamplerState<>::GetRHI();
				ParametersPS.BaseColor = DrawInfo.BaseColor;
				ParametersPS.MultiplyColor = DrawInfo.MultiplyColor;
				ParametersPS.ScreenColor = DrawInfo.ScreenColor;
				ParametersPS.MaskTexture = DrawInfo.MaskTexture->TextureRHI;
				ParametersPS.MaskSampler = TStaticSamplerState<>::GetRHI();
				ParametersPS.Offset = DrawInfo.Offset;
				ParametersPS.Channel = DrawInfo.Channel;

				SetShaderParameters(RHICmdList, PixelShaderMasked, PixelShaderMasked.GetPixelShader(), ParametersPS);
			}
		}
		else
		{
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShaderNormal.GetPixelShader();

			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			FCubismMeshPS<ECubismMeshPSType::Normal>::FParameters ParametersPS;

			ParametersPS.MainTexture = DrawInfo.MainTexture->TextureRHI;
			ParametersPS.MainSampler = TStaticSamplerState<>::GetRHI();
			ParametersPS.BaseColor = DrawInfo.BaseColor;
			ParametersPS.MultiplyColor = DrawInfo.MultiplyColor;
			ParametersPS.ScreenColor = DrawInfo.ScreenColor;

			SetShaderParameters(RHICmdList, PixelShaderNormal, PixelShaderNormal.GetPixelShader(), ParametersPS);
		}

		{
			FRHIResourceCreateInfo VertexBufferInfo(TEXT("VertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			FBufferRHIRef VertexBuffer = RHICmdList.CreateVertexBuffer(sizeof(FCubismMeshVertex) * NumVerts, BUF_Volatile, VertexBufferInfo);
			#else
			FBufferRHIRef VertexBuffer = RHICreateVertexBuffer(sizeof(FCubismMeshVertex) * NumVerts, BUF_Volatile, VertexBufferInfo);
			#endif
			void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer, 0, sizeof(FCubismMeshVertex) * NumVerts, RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, DrawInfo.Vertices.GetData(), sizeof(FCubismMeshVertex) * NumVerts);
			RHICmdList.UnlockBuffer(VertexBuffer);

			// Bind the vertex buffers
			RHICmdList.SetStreamSource(0, VertexBuffer, 0);

			// Release the vertex buffer
			VertexBuffer.SafeRelease();
		}

		{
			FRHIResourceCreateInfo IndexBufferInfo(TEXT("IndexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			FBufferRHIRef IndexBuffer = RHICmdList.CreateIndexBuffer(sizeof(uint16), sizeof(uint16) * NumIndices, BUF_Volatile, IndexBufferInfo);
			#else
			FBufferRHIRef IndexBuffer = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * NumIndices, BUF_Volatile, IndexBufferInfo);
			#endif
			void* IndexBufferData = RHICmdList.LockBuffer(IndexBuffer, 0, sizeof(uint16) * NumIndices, RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, DrawInfo.Indices.GetData(), sizeof(uint16) * NumIndices);
			RHICmdList.UnlockBuffer(IndexBuffer);

			// Issue the draw call
			RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, NumVerts, 0, NumIndices / 3, 1);

			// Release the index buffer
			IndexBuffer.SafeRelease();
		}
	}

	RHICmdList.EndRenderPass();
}

IMPLEMENT_GLOBAL_SHADER(FCubismMeshVS, "/Plugin/Live2DCubismSDK/Private/CubismMesh.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FCubismMeshPS<ECubismMeshPSType::Normal>        , "/Plugin/Live2DCubismSDK/Private/CubismMesh.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FCubismMeshPS<ECubismMeshPSType::Masked>        , "/Plugin/Live2DCubismSDK/Private/CubismMesh.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FCubismMeshPS<ECubismMeshPSType::InvertedMasked>, "/Plugin/Live2DCubismSDK/Private/CubismMesh.usf", "MainPS", SF_Pixel);
