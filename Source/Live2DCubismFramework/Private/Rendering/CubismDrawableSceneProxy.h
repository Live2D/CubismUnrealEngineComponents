/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "SceneView.h"
#include "MeshBatch.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialRenderProxy.h"
#include "SceneInterface.h"
#include "Engine.h"
#include "Math/Color.h"
#include "VertexFactory.h"
#include "CubismRenderingResource.h"

#include <array>

/**
 * A representation of a UCubismDrawableComponent on the rendering thread.
 */
class FCubismDrawableSceneProxy : public FPrimitiveSceneProxy
{
public:
	static constexpr uint32 kNumMultipleBufferingResources = 3;

	FCubismDrawableSceneProxy(const TObjectPtr<UCubismDrawableComponent>& Drawable, FCubismDrawableDynamicMeshData InDynamicData)
		: FPrimitiveSceneProxy(Drawable)
		, DynamicData(InDynamicData)
		, MaterialInstance(Drawable->GetMaterial(0))
		, MaterialRelevance(Drawable->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		ENQUEUE_RENDER_COMMAND(FCubismDrawableSceneProxy_Ctor)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				UpdateDynamicData(RHICmdList, DynamicData);
			}
		);
	}

	virtual ~FCubismDrawableSceneProxy() override
	{
		if (IsInitializedDrawableResources)
		{
			ClearDrawableResources();

			IsInitializedDrawableResources = false;
		}
	}

	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FMaterialRenderProxy* GetMaterialRenderProxy(FMeshElementCollector& Collector, const bool bWireframe) const
	{
		if (bWireframe)
		{
			FColoredMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial->GetRenderProxy(),
				FLinearColor(0.0f, 0.5f, 1.0f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

			return WireframeMaterialInstance;
		}


		return MaterialInstance->GetRenderProxy();
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector
	) const override
	{
		if (!DynamicData.ExistsAllElements())
		{
			return;
		}


		FCubismDrawableResource* CurrentResource = GetCurrentResource();


		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
		FMaterialRenderProxy* MaterialRenderProxy = GetMaterialRenderProxy(Collector, bWireframe);


		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (!(VisibilityMap & 1 << ViewIndex))
			{
				continue;
			}


			const FSceneView* View = Views[ViewIndex];


			FMeshBatch& Mesh = Collector.AllocateMesh();
			{
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.bDisableBackfaceCulling = DynamicData.bTwoSided;
				Mesh.Type = PT_TriangleList;
				Mesh.VertexFactory = CurrentResource->GetLocalVertexFactoryPointer();
				Mesh.MaterialRenderProxy = MaterialRenderProxy;
			}


			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			{
				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();


#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
				DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, AlwaysHasVelocity());

#else
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, AlwaysHasVelocity());

#endif


				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
				BatchElement.IndexBuffer = &CurrentResource->GetIndexBuffer();
				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = CurrentResource->NumIndices() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = CurrentResource->NumVertices() - 1;
			}

			Collector.AddMesh(ViewIndex, Mesh);
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

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	void UpdateDynamicData(FRHICommandListImmediate& RHICmdList, const FCubismDrawableDynamicMeshData& NewDynamicData)
	{
		DynamicData = NewDynamicData;


		if (!DynamicData.ExistsAllElements())
		{
			return;
		}


		if (!IsInitializedDrawableResources)
		{
			ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
			MakeDrawableResources(RHICmdList, FeatureLevel);


			IsInitializedDrawableResources = true;

			return;
		}


		GetCurrentResource()->UpdateBuffer(DynamicData);
	}

private:
	static int32 GetCurrentBufferIndex()
	{
		return GFrameNumberRenderThread % kNumMultipleBufferingResources;
	}

	FCubismDrawableResource* GetCurrentResource() const
	{
		const int32 BufferIndex = GetCurrentBufferIndex();
		return DrawableResources[BufferIndex];
	}

	void MakeDrawableResources(FRHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type FeatureLevel)
	{
		for (auto& Resource : DrawableResources)
		{
			Resource = new FCubismDrawableResource(DynamicData, FeatureLevel);

			Resource->InitResource(RHICmdList);
		}
	}

	void ClearDrawableResources()
	{
		for (auto& Resource : DrawableResources)
		{
			if (!Resource)
			{
				continue;
			}


			Resource->ReleaseResource();

			delete Resource;
			Resource = nullptr;
		}
	}

	/** Dynamic mesh data for the drawable. */
	FCubismDrawableDynamicMeshData DynamicData = {};


	/** The material instance to use for rendering. */
	UMaterialInterface* MaterialInstance = nullptr;

	/** The material relevance for the drawable. */
	FMaterialRelevance MaterialRelevance = {};

	/** Dynamic rendering resources */
	std::array<FCubismDrawableResource*, kNumMultipleBufferingResources> DrawableResources = {};
	bool IsInitializedDrawableResources = false;
};
