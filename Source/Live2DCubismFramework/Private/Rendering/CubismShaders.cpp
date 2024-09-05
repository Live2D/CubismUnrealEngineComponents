/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismShaders.h"

#include "ShaderParameterUtils.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "BatchedElements.h"
#include "RenderResource.h"
#include "GlobalShader.h"

/*** Cubism Mask Shader ***/

class FCubismMeshMaskVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshMaskVS);

	FCubismMeshMaskVS() {}
	FCubismMeshMaskVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Scale.Bind(Initializer.ParameterMap, TEXT("Scale"));
		Offset.Bind(Initializer.ParameterMap, TEXT("Offset"));
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList, const TShaderRHIParamRef ShaderRHI, const FVector4& InOffset)
	{
		#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, Offset, FVector4f(InOffset));
		RHICmdList.SetBatchedShaderParameters(ShaderRHI, BatchedParameters);
		#else
		SetShaderValue(RHICmdList, ShaderRHI, Offset, FVector4f(InOffset));
		#endif
	}

protected:
	LAYOUT_FIELD(FShaderParameter, Scale);
	LAYOUT_FIELD(FShaderParameter, Offset);
};

class FCubismMeshMaskPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshMaskPS);
	SHADER_USE_PARAMETER_STRUCT(FCubismMeshMaskPS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector4f, Channel)
		SHADER_PARAMETER_TEXTURE(Texture2D, MainTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, MainSampler)
	END_SHADER_PARAMETER_STRUCT()
};

void FCubismMaskBatchedElementParameters::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{
	TShaderMapRef<FCubismMeshMaskVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FCubismMeshMaskPS> PixelShader(GetGlobalShaderMap(InFeatureLevel));

	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();

	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

	VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader(), Offset);

	FCubismMeshMaskPS::FParameters ShaderParameters;
	ShaderParameters.Channel = (FVector4f)Channel;
	ShaderParameters.MainTexture = MainTexture->TextureRHI;
	ShaderParameters.MainSampler = TStaticSamplerState<>::GetRHI();

	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), ShaderParameters);
}

IMPLEMENT_GLOBAL_SHADER(FCubismMeshMaskVS, "/Plugin/Live2DCubismSDK/Private/CubismMeshMask.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FCubismMeshMaskPS, "/Plugin/Live2DCubismSDK/Private/CubismMeshMask.usf", "MainPS", SF_Pixel);
