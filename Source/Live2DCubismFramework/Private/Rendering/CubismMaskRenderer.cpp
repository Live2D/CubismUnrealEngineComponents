/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismMaskRenderer.h"

#include "PipelineStateCache.h"
#include "RenderGraphUtils.h"
#include "RHIStaticStates.h"
#include "RHIResources.h"

class FCubismMeshMaskVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;


	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FVertexDeclarationElementList Elements;
		constexpr uint16 Stride = sizeof(FCubismMaskRenderer::FMeshVertexData);

		Elements.Add(FVertexElement(0U, STRUCT_OFFSET(FCubismMaskRenderer::FMeshVertexData, Position), VET_Float2, 0U, Stride));
		Elements.Add(FVertexElement(0U, STRUCT_OFFSET(FCubismMaskRenderer::FMeshVertexData, UV), VET_Float2, 1U, Stride));


		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FCubismMeshMaskVertexDeclaration> GCubismMeshMaskVertexDeclaration;


class FCubismMaskShader_VS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMaskShader_VS);


	FCubismMaskShader_VS() = default;

	explicit FCubismMaskShader_VS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Offset.Bind(Initializer.ParameterMap, TEXT("Offset"));
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	void SetMaskOffset(FRHIBatchedShaderParameters& BatchedShaderParameters, const FVector4& InOffset) const
	{
		SetShaderValue(BatchedShaderParameters, Offset, static_cast<FVector4f>(InOffset));
	}

private:
	LAYOUT_FIELD(FShaderParameter, Offset);
};

class FCubismMaskShader_PS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMaskShader_PS);

	SHADER_USE_PARAMETER_STRUCT(FCubismMaskShader_PS, FGlobalShader);


	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector4f, Channel)
		SHADER_PARAMETER_TEXTURE(Texture2D, MainTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, MainSampler)
	END_SHADER_PARAMETER_STRUCT()


	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FCubismMaskShader_VS, "/Plugin/Live2DCubismSDK/Private/CubismMaskShader.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FCubismMaskShader_PS, "/Plugin/Live2DCubismSDK/Private/CubismMaskShader.usf", "MainPS", SF_Pixel);


FCubismMaskRenderer::FCubismMaskRenderer(const int64 InNumVertices, const int64 InNumIndices)
	: NumVertices(InNumVertices)
	, NumIndices(InNumIndices)
	, bIsInitialized(false)
{
}

FCubismMaskRenderer::~FCubismMaskRenderer()
{
	if (!bIsInitialized)
	{
		return;
	}


	ReleaseResource();
}

void FCubismMaskRenderer::InitResource_RenderThread(FRHICommandListBase& RHICmdList)
{
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION >= 6)

	{
		FRHIBufferCreateDesc VertexBufferDesc(TEXT("CubismMaskVertexBuffer"), sizeof(FMeshVertexData) * NumVertices, sizeof(FMeshVertexData), EBufferUsageFlags::VertexBuffer);

		VertexBufferDesc.AddUsage(BUF_Dynamic);
		VertexBufferDesc.InitialState = ERHIAccess::VertexOrIndexBuffer;


		MaskMeshBuffer.VertexBuffer = RHICmdList.CreateBuffer(VertexBufferDesc);
	}


	{
		FRHIBufferCreateDesc IndexBufferDesc(TEXT("CubismMaskIndexBuffer"), sizeof(uint16) * NumIndices, sizeof(uint16), EBufferUsageFlags::IndexBuffer);

		IndexBufferDesc.AddUsage(BUF_Dynamic);
		IndexBufferDesc.InitialState = ERHIAccess::VertexOrIndexBuffer;


		MaskMeshBuffer.IndexBuffer = RHICmdList.CreateBuffer(IndexBufferDesc);
	}

#else

	{
		FRHIResourceCreateInfo VertexBufferInfo(TEXT("CubismMaskVertexBuffer"));

		MaskMeshBuffer.VertexBuffer = RHICmdList.CreateVertexBuffer(sizeof(FMeshVertexData) * NumVertices, BUF_Dynamic, VertexBufferInfo);
	}

	{

		FRHIResourceCreateInfo IndexBufferInfo(TEXT("CubismMaskIndexBuffer"));

		MaskMeshBuffer.IndexBuffer = RHICmdList.CreateIndexBuffer(sizeof(uint16), sizeof(uint16) * NumIndices, BUF_Dynamic, IndexBufferInfo);
	}

#endif

	bIsInitialized = true;
}

void FCubismMaskRenderer::ReleaseResource()
{
	ENQUEUE_RENDER_COMMAND(ReleaseCubismMaskBuffers)(
		[VertexBuffer = MaskMeshBuffer.VertexBuffer, IndexBuffer = MaskMeshBuffer.IndexBuffer](FRHICommandListImmediate& RHICmdList) mutable
		{

			VertexBuffer.SafeRelease();

			IndexBuffer.SafeRelease();

		});
}

void FCubismMaskRenderer::DrawMesh_RenderThread(FRHICommandList& RHICmdList, const FDrawableInfo& MaskDrawableInfo)
{
	if (!bIsInitialized)
	{
		InitResource_RenderThread(RHICmdList);
	}


	const FBufferRHIRef& VertexBuffer = MaskMeshBuffer.VertexBuffer;
	const FBufferRHIRef& IndexBuffer = MaskMeshBuffer.IndexBuffer;

	{
		const size_t VertexBufferSizeBytes = sizeof(FMeshVertexData) * NumVertices;
		void* VertexBufferData =
			RHICmdList.LockBuffer(VertexBuffer, 0U, static_cast<uint32>(VertexBufferSizeBytes), RLM_WriteOnly);

		FMemory::Memcpy(VertexBufferData, MaskDrawableInfo.Vertices.GetData(), VertexBufferSizeBytes);

		RHICmdList.UnlockBuffer(VertexBuffer);
	}

	{
		const size_t IndexBufferSizeBytes = sizeof(uint16) * NumIndices;
		void* IndexBufferData = 
			RHICmdList.LockBuffer(IndexBuffer, 0U, static_cast<uint32>(IndexBufferSizeBytes), RLM_WriteOnly);

		FMemory::Memcpy(IndexBufferData, MaskDrawableInfo.Indices.GetData(), IndexBufferSizeBytes);

		RHICmdList.UnlockBuffer(IndexBuffer);
	}


	RHICmdList.SetStreamSource(0U, VertexBuffer, 0U);


	RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0U, static_cast<uint32>(NumVertices), 0U, static_cast<uint32>(NumIndices / 3LL), 1U);
}

void FCubismMaskRenderer::DrawMeshes_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* MaskRenderTarget, const TArray<FDrawableInfo>& MaskDrawableInfoArray, ERHIFeatureLevel::Type FeatureLevel)
{
	FRHIRenderPassInfo RenderPassInfo(MaskRenderTarget->GetRenderTargetTexture(), ERenderTargetActions::Clear_Store);
	
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("DrawCubismMeshMask"));


	// Set viewport and scissor rect to cover the entire render target.
	{
		const uint32 SizeX = MaskRenderTarget->GetSizeX();
		const uint32 SizeY = MaskRenderTarget->GetSizeY();

		RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, static_cast<float>(SizeX), static_cast<float>(SizeY), 1.0f);
		RHICmdList.SetScissorRect(false, 0U, 0U, 0U, 0U);
	}


	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FCubismMaskShader_VS> VertexShader(ShaderMap);
	TShaderMapRef<FCubismMaskShader_PS> PixelShader(ShaderMap);
	FGraphicsPipelineStateInitializer GraphicsPSOInit;

	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	{
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismMeshMaskVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	}
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0U);


	for (const FDrawableInfo& MaskDrawableInfo : MaskDrawableInfoArray)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		

		VertexShader->SetMaskOffset(BatchedParameters, MaskDrawableInfo.Offset);
		
		RHICmdList.SetBatchedShaderParameters(VertexShader.GetVertexShader(), BatchedParameters);


		FCubismMaskShader_PS::FParameters ParametersPS;

		ParametersPS.Channel = static_cast<FVector4f>(MaskDrawableInfo.Channel);
		ParametersPS.MainTexture = MaskDrawableInfo.MainTexture->TextureRHI;
		ParametersPS.MainSampler = TStaticSamplerState<>::GetRHI();


		SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), ParametersPS);


		MaskDrawableInfo.Renderer->DrawMesh_RenderThread(RHICmdList, MaskDrawableInfo);
	}


	RHICmdList.EndRenderPass();
}
