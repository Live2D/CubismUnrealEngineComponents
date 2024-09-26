/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "Materials/Material.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
#include "Materials/MaterialRenderProxy.h"
#endif

/**
 * Dynamic mesh data for a drawable.
 */
struct FCubismDrawableDynamicMeshData
{
	int32 Index;
	TArray<uint32> Indices;
	FColor Color;
	TArray<FVector3f> Positions;
	TArray<FVector2f> UVs;
	bool bTwoSided;
};

/**
 * A representation of a UCubismDrawableComponent on the rendering thread.
 */
class FCubismDrawableSceneProxy : public FPrimitiveSceneProxy
{
public:
	FCubismDrawableSceneProxy(const TObjectPtr<UCubismDrawableComponent>& Drawable)
		: FPrimitiveSceneProxy(Drawable)
		, MaterialInstance(Drawable->GetMaterial(0))
		, MaterialRelevance(Drawable->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
	}

	virtual ~FCubismDrawableSceneProxy() { }

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector
	) const override
	{
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FMaterialRenderProxy* MaterialProxy = nullptr;
		if (bWireframe)
		{
			FColoredMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial->GetRenderProxy(),
				FLinearColor(0.0f, 0.5f, 1.0f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = MaterialInstance->GetRenderProxy();
		}

		TArray<FDynamicMeshVertex> Vertices;
		for (int32 i = 0; i < DynamicData.Positions.Num(); i++)
		{
			FDynamicMeshVertex Vertex;

			Vertex.SetTangents(FVector3f(1.0f,0.0f,0.0f), FVector3f(0.0f,1.0f,0.0f), FVector3f(0.0f,0.0f,1.0f));
			Vertex.Color = DynamicData.Color;
			Vertex.Position = DynamicData.Positions[i];
			Vertex.TextureCoordinate[0] = DynamicData.UVs[i];

			Vertices.Add(Vertex);
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];

				FDynamicMeshBuilder Builder(View->GetFeatureLevel());
				Builder.AddVertices(Vertices);
				Builder.AddTriangles(DynamicData.Indices);
				Builder.GetMesh(
					GetLocalToWorld(),
					MaterialProxy,
					SDPG_World,
					DynamicData.bTwoSided,
					false,
					ViewIndex,
					Collector
				);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override { return !MaterialRelevance.bDisableDepthTest; }

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

public:
	/** Dynamic mesh data for the drawable. */
	FCubismDrawableDynamicMeshData DynamicData;

private:
	/** The material instance to use for rendering. */
	UMaterialInterface* MaterialInstance;

	/** The material relevance for the drawable. */
	FMaterialRelevance MaterialRelevance;
};
