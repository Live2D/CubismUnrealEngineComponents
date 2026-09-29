/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismDrawableDynamicMeshData.h"
#include "CubismVertexBuffer.h"
#include "CubismUshortIndexBuffer.h"

#include "Materials/MaterialRenderProxy.h"
#include "DataDrivenShaderPlatformInfo.h"
/**
 * Vertex buffer for a drawable.
 */
class FCubismDrawableResource : public FRenderResource
{
public:
	FCubismDrawableResource(const FCubismDrawableDynamicMeshData& DynamicData, ERHIFeatureLevel::Type FeatureLevel) : LocalVertexFactory({ FeatureLevel, "CubismLocalVertexFactory" })
	{
		PositionBuffer.Init(DynamicData.Positions, TEXT("CubismPositionVertexBuffer"), true);
		UVBuffer.Init(DynamicData.UVs, TEXT("CubismUvVertexBuffer"), true);
		TangentXBuffer.Init(FPackedNormal(FVector4f(1.0f, 0.0f, 0.0f, 1.0f)), DynamicData.Positions.Num(), TEXT("CubismTangentVertexBuffer"), true);
		TangentZBuffer.Init(FPackedNormal(FVector4f(0.0f, 0.0f, 1.0f, 0.0f)), DynamicData.Positions.Num(), TEXT("CubismNormalVertexBuffer"), true);

		IndexBuffer.Init(DynamicData.Indices, TEXT("CubismIndexBuffer"), true);
	}

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCubismDrawableResource::InitRHI)


		PositionBuffer.InitResource(RHICmdList);
		PositionBuffer.InitRHI(RHICmdList);

		UVBuffer.InitResource(RHICmdList);
		UVBuffer.InitRHI(RHICmdList);

		TangentXBuffer.InitResource(RHICmdList);
		TangentXBuffer.InitRHI(RHICmdList);

		TangentZBuffer.InitResource(RHICmdList);
		TangentZBuffer.InitRHI(RHICmdList);


		IndexBuffer.InitResource(RHICmdList);
		IndexBuffer.InitRHI(RHICmdList);


		FLocalVertexFactory::FDataType Data;
		MakeLocalVertexFactoryData(Data);
		
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		LocalVertexFactory.SetData(RHICmdList, Data);
#else
		LocalVertexFactory.SetData(Data);
#endif
		LocalVertexFactory.InitResource(RHICmdList);
	}

	virtual void ReleaseRHI() override
	{
		LocalVertexFactory.ReleaseResource();


		TangentZBuffer.ReleaseResource();
		TangentXBuffer.ReleaseResource();
		UVBuffer.ReleaseResource();
		PositionBuffer.ReleaseResource();
		

		IndexBuffer.ReleaseResource();
	}

	void UpdateBuffer(const FCubismDrawableDynamicMeshData& dynamicMeshData)
	{
		const uint32 PositionSizeInBytes = FCubismVertexBuffer<FVector3f>::Stride * dynamicMeshData.Positions.Num();
		const uint32 UVsSizeInBytes = FCubismVertexBuffer<FVector2f>::Stride * dynamicMeshData.UVs.Num();
		const uint32 IndicesSizeInBytes = FCubismUshortIndexBuffer::Stride * dynamicMeshData.Indices.Num();


		ensure(PositionBuffer.GetNumVertices() == dynamicMeshData.Positions.Num());
		ensure(UVBuffer.GetNumVertices() == dynamicMeshData.UVs.Num());
		ensure(IndexBuffer.GetNumIndices() == dynamicMeshData.Indices.Num());


		ENQUEUE_RENDER_COMMAND(FCubismRenderingResource_UpdateBuffer)(
			[this, dynamicMeshData, PositionSizeInBytes, UVsSizeInBytes, IndicesSizeInBytes](FRHICommandListImmediate& RHICmdList)
			{
				{
					void* PositionBufferData = RHICmdList.LockBuffer(PositionBuffer.VertexBufferRHI, 0, PositionSizeInBytes, RLM_WriteOnly);
					FMemory::Memcpy(PositionBufferData, dynamicMeshData.Positions.GetData(), PositionSizeInBytes);
					RHICmdList.UnlockBuffer(PositionBuffer.VertexBufferRHI);
				}
				{
					void* UVBufferData = RHICmdList.LockBuffer(UVBuffer.VertexBufferRHI, 0, UVsSizeInBytes, RLM_WriteOnly);
					FMemory::Memcpy(UVBufferData, dynamicMeshData.UVs.GetData(), UVsSizeInBytes);
					RHICmdList.UnlockBuffer(UVBuffer.VertexBufferRHI);
				}
				{
					void* IndexBufferData = RHICmdList.LockBuffer(IndexBuffer.IndexBufferRHI, 0, IndicesSizeInBytes, RLM_WriteOnly);
					FMemory::Memcpy(IndexBufferData, dynamicMeshData.Indices.GetData(), IndicesSizeInBytes);
					RHICmdList.UnlockBuffer(IndexBuffer.IndexBufferRHI);
				}
			}
		);


		FMemory::Memcpy(PositionBuffer.GetVertexData(), dynamicMeshData.Positions.GetData(), PositionSizeInBytes);
		FMemory::Memcpy(UVBuffer.GetVertexData(), dynamicMeshData.UVs.GetData(), UVsSizeInBytes);
		FMemory::Memcpy(IndexBuffer.GetIndexData(), dynamicMeshData.Indices.GetData(), IndicesSizeInBytes);
	}

	const FLocalVertexFactory* GetLocalVertexFactoryPointer() const
	{
		return &LocalVertexFactory;
	}

	uint32 NumVertices() const
	{
		return PositionBuffer.GetNumVertices();
	}

	uint32 NumIndices() const
	{
		return IndexBuffer.GetNumIndices();
	}

	const FCubismUshortIndexBuffer& GetIndexBuffer()
	{
		return IndexBuffer;
	}

private:
	void MakeLocalVertexFactoryData(FLocalVertexFactory::FDataType& Data) const
	{
		Data.PositionComponent = FVertexStreamComponent(&PositionBuffer, 0, FCubismVertexBuffer<FVector3f>::Stride, VET_Float3);
		Data.PositionComponentSRV = PositionBuffer.GetSRV();
		Data.TextureCoordinates.Reset();
		Data.TextureCoordinates.Add(FVertexStreamComponent(&UVBuffer, 0, FCubismVertexBuffer<FVector2f>::Stride, VET_Float2));
		Data.TextureCoordinatesSRV = UVBuffer.GetSRV();
		Data.TangentsSRV = GNullVertexBuffer.VertexBufferSRV;
		Data.TangentBasisComponents[0] = FVertexStreamComponent(&TangentXBuffer, 0, FCubismVertexBuffer<FPackedNormal>::Stride, VET_PackedNormal);
		Data.TangentBasisComponents[1] = FVertexStreamComponent(&TangentZBuffer, 0, FCubismVertexBuffer<FPackedNormal>::Stride, VET_PackedNormal);

		Data.ColorComponentsSRV = GNullColorVertexBuffer.VertexBufferSRV;
		Data.NumTexCoords = 1;
	}

	FCubismVertexBuffer<FVector3f> PositionBuffer = {};
	FCubismVertexBuffer<FVector2f> UVBuffer = {};
	FCubismVertexBuffer<FPackedNormal> TangentXBuffer = {};
	FCubismVertexBuffer<FPackedNormal> TangentZBuffer = {};

	FCubismUshortIndexBuffer IndexBuffer = {};

	FLocalVertexFactory LocalVertexFactory;
};
