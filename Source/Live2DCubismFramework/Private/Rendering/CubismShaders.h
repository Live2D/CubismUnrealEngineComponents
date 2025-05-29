/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismDrawableComponent.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "TextureResource.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Runtime/Launch/Resources/Version.h"

enum ECubismMeshPSType : int32
{
	Normal,
	Masked,
	InvertedMasked,
};

struct FCubismMeshVertex
{
	FVector2f Position; // ATTRIBUTE0
	FVector2f UV;       // ATTRIBUTE1
};

struct FDrawInfo
{
	ECubismDrawableBlendMode BlendMode;
	int32 RenderOrder;
	TArray<uint16> Indices;
	TArray<FCubismMeshVertex> Vertices;
	FTexture* MainTexture;
	FVector4f BaseColor;
	FVector4f MultiplyColor;
	FVector4f ScreenColor;
	bool IsMasked;
	bool InvertedMask;
	FTexture* MaskTexture;
	FVector4f Offset;
	FVector4f Channel;
};

/*** Cubism Shader ***/

void DrawCubismMesh_RenderThread(FRHICommandList& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, const TArray<FDrawInfo>& DrawInfos);

class FCubismMeshVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshVS);

	FCubismMeshVS() {}
	FCubismMeshVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{ }

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

template<ECubismMeshPSType CubismMeshPSType>
class FCubismMeshPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCubismMeshPS);
	SHADER_USE_PARAMETER_STRUCT(FCubismMeshPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, MainTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, MainSampler)
		SHADER_PARAMETER(FVector4f, BaseColor)
		SHADER_PARAMETER(FVector4f, MultiplyColor)
		SHADER_PARAMETER(FVector4f, ScreenColor)
		SHADER_PARAMETER_TEXTURE(Texture2D, MaskTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, MaskSampler)
		SHADER_PARAMETER(FVector4f, Offset)
		SHADER_PARAMETER(FVector4f, Channel)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("IS_MASKED"), CubismMeshPSType == ECubismMeshPSType::Masked || CubismMeshPSType == ECubismMeshPSType::InvertedMasked);
		OutEnvironment.SetDefine(TEXT("INVERTED"), CubismMeshPSType == ECubismMeshPSType::InvertedMasked);
	}
};
