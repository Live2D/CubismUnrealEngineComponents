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

/**
 * A representation of a UCubismDrawableComponent on the rendering thread.
 */
class FCubismDrawableSceneProxy : public FPrimitiveSceneProxy
{
public:
	FCubismDrawableSceneProxy(const TObjectPtr<UCubismDrawableComponent>& Drawable, FCubismDrawableDynamicMeshData InDynamicData)
		: FPrimitiveSceneProxy(Drawable)
		, DynamicData(InDynamicData)
		, MaterialInstance(Drawable->GetMaterial(0))
		, MaterialRelevance(Drawable->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, VertexBuffer(nullptr)
		, IndexBuffer(nullptr)
		, VertexFactory(nullptr)
	{
		ENQUEUE_RENDER_COMMAND(InitCubismDrawableSceneProxy)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				UpdateDynamicData(RHICmdList, DynamicData);
			}
		);
	}

	virtual ~FCubismDrawableSceneProxy() 
	{ 
		if (VertexFactory)
		{
			VertexFactory->ReleaseResource();
			delete VertexFactory;
		}
		if (VertexBuffer)
		{
			VertexBuffer->ReleaseResource();
			delete VertexBuffer;
		}
		if (IndexBuffer)
		{
			IndexBuffer->ReleaseResource();
			delete IndexBuffer;
		}
	}

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
		if (DynamicData.Positions.Num() == 0 || DynamicData.UVs.Num() == 0 || DynamicData.Indices.Num() == 0)
		{
			return;
		}

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FMaterialRenderProxy* MaterialRenderProxy = nullptr;
		if (bWireframe)
		{
			FColoredMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial->GetRenderProxy(),
				FLinearColor(0.0f, 0.5f, 1.0f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
			MaterialRenderProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialRenderProxy = MaterialInstance->GetRenderProxy();
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];

				FMeshBatch& Mesh = Collector.AllocateMesh();
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.bDisableBackfaceCulling = DynamicData.bTwoSided;
				Mesh.Type = PT_TriangleList;

				Mesh.VertexFactory = VertexFactory;
				Mesh.MaterialRenderProxy = MaterialRenderProxy;

				FMeshBatchElement& BatchElement = Mesh.Elements[0];

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
				DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, AlwaysHasVelocity());
				#else
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, AlwaysHasVelocity());
				#endif
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.IndexBuffer = IndexBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer->Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffer->Positions.Num() - 1;

				Collector.AddMesh(ViewIndex, Mesh);
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

	void UpdateDynamicData(FRHICommandListImmediate& RHICmdList, const FCubismDrawableDynamicMeshData& NewDynamicData)
	{
		DynamicData = NewDynamicData;

		if (DynamicData.Positions.Num() > 0 && DynamicData.UVs.Num() > 0)
		{
			if (VertexBuffer)
			{
				VertexBuffer->UpdateBuffer(DynamicData.Positions, DynamicData.UVs);
			}
			else
			{
				VertexBuffer = new FCubismDrawableVertexBuffer(DynamicData);
				#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
				VertexBuffer->InitResource(RHICmdList);
				#else
				VertexBuffer->InitResource();
				#endif
			}
		}

		if (DynamicData.Indices.Num() > 0)
		{
			if (IndexBuffer)
			{
				IndexBuffer->UpdateBuffer(DynamicData.Indices);
			}
			else
			{
				IndexBuffer = new FCubismDrawableIndexBuffer(DynamicData);
				#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
				IndexBuffer->InitResource(RHICmdList);
				#else
				IndexBuffer->InitResource();
				#endif
			}
		}

		if (!VertexFactory && VertexBuffer)
		{
			VertexFactory = new FCubismDrawableVertexFactory(GetScene().GetFeatureLevel(), VertexBuffer);
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			VertexFactory->InitResource(RHICmdList);
			#else
			VertexFactory->InitResource();
			#endif
		}
	}

public:
	/** Dynamic mesh data for the drawable. */
	FCubismDrawableDynamicMeshData DynamicData;

private:
	/** The material instance to use for rendering. */
	UMaterialInterface* MaterialInstance;

	/** The material relevance for the drawable. */
	FMaterialRelevance MaterialRelevance;

	/** Dynamic rendering resources */
	mutable FCubismDrawableVertexBuffer* VertexBuffer;
	mutable FCubismDrawableIndexBuffer* IndexBuffer;
	mutable FCubismDrawableVertexFactory* VertexFactory;
};
