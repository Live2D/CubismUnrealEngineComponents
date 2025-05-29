/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismMaskShaders.h"

#include "ClearQuad.h"
#include "PipelineStateCache.h"
#include "RenderGraphResources.h"
#include "RHIStaticStates.h"

class FCubismMeshMaskVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FCubismMeshMaskVertexDeclaration() {}

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	#else
	virtual void InitRHI() override
	#endif
	{
		FVertexDeclarationElementList Elements;
		uint16 Stride = sizeof(FCubismMeshMaskVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismMeshMaskVertex, Position), VET_Float2, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismMeshMaskVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FCubismMeshMaskVertexDeclaration> GCubismMeshMaskVertexDeclaration;

void DrawCubismMeshMask_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, const TArray<FMaskDrawInfo>& MaskDrawInfos)
{
	FRHIRenderPassInfo RPInfo(RenderTargetResource->GetRenderTargetTexture(), ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawCubismMeshMask"));

	// Clear the render target.
	DrawClearQuad(RHICmdList, FLinearColor::Transparent);

	TShaderMapRef<FCubismMeshMaskVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FCubismMeshMaskPS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismMeshMaskVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;

	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

	for (const FMaskDrawInfo& DrawInfo : MaskDrawInfos)
	{
		#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		VertexShader->SetParameters(BatchedParameters, DrawInfo.Offset);
		RHICmdList.SetBatchedShaderParameters(VertexShader.GetVertexShader(), BatchedParameters);
		#else
		VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader(), DrawInfo.Offset);
		#endif

		FCubismMeshMaskPS::FParameters ParametersPS;

		ParametersPS.Channel = (FVector4f)DrawInfo.Channel;
		ParametersPS.MainTexture = DrawInfo.MainTexture->TextureRHI;
		ParametersPS.MainSampler = TStaticSamplerState<>::GetRHI();

		SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), ParametersPS);

		int32 NumVerts = DrawInfo.Vertices.Num();
		int32 NumIndices = DrawInfo.Indices.Num();

		{
			FRHIResourceCreateInfo VertexBufferInfo(TEXT("MaskVertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			FBufferRHIRef VertexBuffer = RHICmdList.CreateVertexBuffer(sizeof(FCubismMeshMaskVertex) * NumVerts, BUF_Volatile, VertexBufferInfo);
			#else
			FBufferRHIRef VertexBuffer = RHICreateVertexBuffer(sizeof(FCubismMeshMaskVertex) * NumVerts, BUF_Volatile, VertexBufferInfo);
			#endif
			void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer, 0, sizeof(FCubismMeshMaskVertex) * NumVerts, RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, DrawInfo.Vertices.GetData(), sizeof(FCubismMeshMaskVertex) * NumVerts);
			RHICmdList.UnlockBuffer(VertexBuffer);

			// Bind the vertex buffers
			RHICmdList.SetStreamSource(0, VertexBuffer, 0);

			// Release the vertex buffer
			VertexBuffer.SafeRelease();
		}

		{
			FRHIResourceCreateInfo IndexBufferInfo(TEXT("MaskIndexBuffer"));
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

IMPLEMENT_GLOBAL_SHADER(FCubismMeshMaskVS, "/Plugin/Live2DCubismSDK/Private/CubismMeshMask.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FCubismMeshMaskPS, "/Plugin/Live2DCubismSDK/Private/CubismMeshMask.usf", "MainPS", SF_Pixel);
