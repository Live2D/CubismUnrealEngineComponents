/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"
#include "TextureResource.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Runtime/Launch/Resources/Version.h"

struct FCubismMeshMaskVertex
{
	FVector2f Position; // ATTRIBUTE0
	FVector2f UV;       // ATTRIBUTE1
};

struct FMaskDrawInfo
{
	TArray<uint16> Indices;
	TArray<FCubismMeshMaskVertex> Vertices;
	FVector4 Offset;
	FVector4 Channel;
	FTexture* MainTexture;
};

/*** Cubism Mask Shader ***/

void DrawCubismMeshMask_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, const TArray<FMaskDrawInfo>& MaskDrawInfos);

class FCubismMeshMaskVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshMaskVS);

	FCubismMeshMaskVS() {}
	FCubismMeshMaskVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Offset.Bind(Initializer.ParameterMap, TEXT("Offset"));
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FVector4& InOffset)
	{
		SetShaderValue(BatchedParameters, Offset, (FVector4f)InOffset);
	}
	#else
	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList, const TShaderRHIParamRef ShaderRHI, const FVector4& InOffset)
	{
		SetShaderValue(RHICmdList, ShaderRHI, Offset, (FVector4f)InOffset);
	}
	#endif

private:
	LAYOUT_FIELD(FShaderParameter, Offset);
};

class FCubismMeshMaskPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshMaskPS);
	SHADER_USE_PARAMETER_STRUCT(FCubismMeshMaskPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector4f, Channel)
		SHADER_PARAMETER_TEXTURE(Texture2D, MainTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, MainSampler)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};
