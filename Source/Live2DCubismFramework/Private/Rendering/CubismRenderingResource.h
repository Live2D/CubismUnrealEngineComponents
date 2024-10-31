/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
#include "Materials/MaterialRenderProxy.h"
#include "DataDrivenShaderPlatformInfo.h"
#else
#include "ShaderParameterStruct.h"
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
#include "PrimitiveUniformShaderParametersBuilder.h"
#else
#include "ResourcePool.h"
#endif

/**
 * Dynamic mesh data for a drawable.
 */
struct FCubismDrawableDynamicMeshData
{
	int32 Index;
	TArray<FVector3f> Positions;
	TArray<FVector2f> UVs;
	TArray<uint16> Indices;
	bool bTwoSided;
};

/**
 * Vertex buffer for a drawable.
 */
class FCubismDrawableVertexBuffer : public FRenderResource
{
public:
	FCubismDrawableVertexBuffer(const FCubismDrawableDynamicMeshData& DynamicData)
		: Positions(DynamicData.Positions)
		, UVs(DynamicData.UVs)
	{ }

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	#else
	virtual void InitRHI() override
	#endif
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCubismDrawableVertexBuffer::InitRHI)

		// Initialize Position Buffer
		{
			const uint32 SizeInBytes = sizeof(FVector3f) * Positions.Num();

			FRHIResourceCreateInfo PositionVertexBufferInfo(TEXT("PositionVertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			PositionBuffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(SizeInBytes, BUF_Volatile, PositionVertexBufferInfo);
			void* PositionBufferData = RHICmdList.LockBuffer(PositionBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
			FMemory::Memcpy(PositionBufferData, Positions.GetData(), SizeInBytes);
			RHICmdList.UnlockBuffer(PositionBuffer.VertexBufferRHI);
			#else
			PositionBuffer.VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Volatile, PositionVertexBufferInfo);
			void* PositionBufferData = RHILockBuffer(PositionBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
			FMemory::Memcpy(PositionBufferData, Positions.GetData(), SizeInBytes);
			RHIUnlockBuffer(PositionBuffer.VertexBufferRHI);
			#endif
		}

		// Initialize UV Buffer
		{
			const uint32 SizeInBytes = sizeof(FVector2f) * UVs.Num();

			FRHIResourceCreateInfo UVVertexBufferInfo(TEXT("UVVertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			UVBuffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(SizeInBytes, BUF_Volatile, UVVertexBufferInfo);
			void* UVBufferData = RHICmdList.LockBuffer(UVBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
			FMemory::Memcpy(UVBufferData, UVs.GetData(), SizeInBytes);
			RHICmdList.UnlockBuffer(UVBuffer.VertexBufferRHI);
			#else
			UVBuffer.VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Volatile, UVVertexBufferInfo);
			void* UVBufferData = RHILockBuffer(UVBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
			FMemory::Memcpy(UVBufferData, UVs.GetData(), SizeInBytes);
			RHIUnlockBuffer(UVBuffer.VertexBufferRHI);
			#endif
		}

		// Initialize Tangent Buffer (unused)
		{
			const uint32 SizeInBytes = 2 * sizeof(FPackedNormal) * Positions.Num();

			FRHIResourceCreateInfo TangentVertexBufferInfo(TEXT("TangentVertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			TangentBuffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(SizeInBytes, BUF_Volatile, TangentVertexBufferInfo);
			#else
			TangentBuffer.VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Volatile, TangentVertexBufferInfo);
			#endif
		}

		// Initialize Color Buffer (unused)
		{
			const uint32 SizeInBytes = sizeof(FColor) * Positions.Num();

			FRHIResourceCreateInfo ColorVertexBufferInfo(TEXT("ColorVertexBuffer"));
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			ColorBuffer.VertexBufferRHI = RHICmdList.CreateVertexBuffer(SizeInBytes, BUF_Volatile, ColorVertexBufferInfo);
			#else
			ColorBuffer.VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Volatile, ColorVertexBufferInfo);
			#endif
		}

		if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
		{
			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			PositionBufferSRV = RHICmdList.CreateShaderResourceView(PositionBuffer.VertexBufferRHI, sizeof(float), PF_R32_FLOAT);
			UVBufferSRV = RHICmdList.CreateShaderResourceView(UVBuffer.VertexBufferRHI, sizeof(FVector2f), PF_G32R32F);
			TangentBufferSRV = RHICmdList.CreateShaderResourceView(TangentBuffer.VertexBufferRHI, 4, PF_R8G8B8A8_SNORM);
			ColorBufferSRV = RHICmdList.CreateShaderResourceView(ColorBuffer.VertexBufferRHI, 4, PF_R8G8B8A8);
			#else
			PositionBufferSRV = RHICreateShaderResourceView(PositionBuffer.VertexBufferRHI, sizeof(float), PF_R32_FLOAT);
			UVBufferSRV = RHICreateShaderResourceView(UVBuffer.VertexBufferRHI, sizeof(FVector2f), PF_G32R32F);
			TangentBufferSRV = RHICreateShaderResourceView(TangentBuffer.VertexBufferRHI, 4, PF_R8G8B8A8_SNORM);
			ColorBufferSRV = RHICreateShaderResourceView(ColorBuffer.VertexBufferRHI, 4, PF_R8G8B8A8);
			#endif
		}
	}

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	void InitResource(FRHICommandListBase& RHICmdList) override
	{
		FRenderResource::InitResource(RHICmdList);

		PositionBuffer.InitResource(RHICmdList);
		UVBuffer.InitResource(RHICmdList);
		TangentBuffer.InitResource(RHICmdList);
		ColorBuffer.InitResource(RHICmdList);
	}
	#else
	void InitResource() override
	{
		FRenderResource::InitResource();

		PositionBuffer.InitResource();
		UVBuffer.InitResource();
		TangentBuffer.InitResource();
		ColorBuffer.InitResource();
	}
	#endif

	void ReleaseResource() override
	{
		FRenderResource::ReleaseResource();

		PositionBuffer.ReleaseResource();
		UVBuffer.ReleaseResource();
		TangentBuffer.ReleaseResource();
		ColorBuffer.ReleaseResource();
	}

	virtual void ReleaseRHI() override
	{
		PositionBuffer.VertexBufferRHI.SafeRelease();
		UVBuffer.VertexBufferRHI.SafeRelease();
		TangentBuffer.VertexBufferRHI.SafeRelease();
		ColorBuffer.VertexBufferRHI.SafeRelease();
	}

	void UpdateBuffer(const TArray<FVector3f>& NewPositions, const TArray<FVector2f>& NewUVs)
	{
		ENQUEUE_RENDER_COMMAND(UpdateVertexBuffer)(
			[this, NewPositions, NewUVs](FRHICommandListImmediate& RHICmdList)
			{
				// Update Position Buffer
				{
					const uint32 SizeInBytes = sizeof(FVector3f) * NewPositions.Num();
					void* PositionBufferData = RHICmdList.LockBuffer(PositionBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
					FMemory::Memcpy(PositionBufferData, NewPositions.GetData(), SizeInBytes);
					RHICmdList.UnlockBuffer(PositionBuffer.VertexBufferRHI);
				}

				// Update UV Buffer
				{
					const uint32 SizeInBytes = sizeof(FVector2f) * NewUVs.Num();
					void* UVBufferData = RHICmdList.LockBuffer(UVBuffer.VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
					FMemory::Memcpy(UVBufferData, NewUVs.GetData(), SizeInBytes);
					RHICmdList.UnlockBuffer(UVBuffer.VertexBufferRHI);
				}
			}
		);
	}

public:
	FVertexBuffer PositionBuffer;
	FVertexBuffer UVBuffer;
	FVertexBuffer TangentBuffer;
	FVertexBuffer ColorBuffer;

	FShaderResourceViewRHIRef PositionBufferSRV;
	FShaderResourceViewRHIRef UVBufferSRV;
	FShaderResourceViewRHIRef TangentBufferSRV;
	FShaderResourceViewRHIRef ColorBufferSRV;

	TArray<FVector3f> Positions;
	TArray<FVector2f> UVs;
};

/**
 * Index buffer for a drawable.
 */
class FCubismDrawableIndexBuffer : public FIndexBuffer
{
public:
	FCubismDrawableIndexBuffer(const FCubismDrawableDynamicMeshData& DynamicData)
		: Indices(DynamicData.Indices)
	{ }

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	#else
	virtual void InitRHI() override
	#endif
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCubismDrawableIndexBuffer::InitRHI)

		const uint32 SizeInBytes = sizeof(uint16) * Indices.Num();

		FRHIResourceCreateInfo IndexBufferInfo(TEXT("IndexBuffer"));
		#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), SizeInBytes, BUF_Volatile, IndexBufferInfo);
		void* IndexBufferData = RHICmdList.LockBuffer(IndexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, Indices.GetData(), SizeInBytes);
		RHICmdList.UnlockBuffer(IndexBufferRHI);
		#else
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), SizeInBytes, BUF_Volatile, IndexBufferInfo);
		void* IndexBufferData = RHILockBuffer(IndexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, Indices.GetData(), SizeInBytes);
		RHIUnlockBuffer(IndexBufferRHI);
		#endif
	}

	virtual void ReleaseRHI() override
	{
		IndexBufferRHI.SafeRelease();
		FIndexBuffer::ReleaseRHI();
	}

	void UpdateBuffer(const TArray<uint16>& NewIndices)
	{
		ENQUEUE_RENDER_COMMAND(UpdateIndexBuffer)(
			[this, NewIndices](FRHICommandListImmediate& RHICmdList)
			{
				const uint32 SizeInBytes = sizeof(uint16) * NewIndices.Num();
				void* IndexBufferData = RHICmdList.LockBuffer(IndexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
				FMemory::Memcpy(IndexBufferData, NewIndices.GetData(), SizeInBytes);
				RHICmdList.UnlockBuffer(IndexBufferRHI);
			}
		);
	}

public:
	TArray<uint16> Indices;
};

/**
 * Vertex factory for a drawable.
 */
class FCubismDrawableVertexFactory : public FLocalVertexFactory
{
public:
	FCubismDrawableVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const FCubismDrawableVertexBuffer* InVertexBuffer)
		: FLocalVertexFactory(InFeatureLevel, "FCubismDrawableVertexFactory")
		, VertexBuffer(InVertexBuffer)
	{ }

	#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	void InitResource(FRHICommandListBase& RHICmdList) override
	#else
	void InitResource() override
	#endif
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCubismDrawableVertexFactory::InitResource)

		{
			FDataType LocalData;

			LocalData.NumTexCoords = 1;
			LocalData.PositionComponentSRV = VertexBuffer->PositionBufferSRV;
			LocalData.TextureCoordinatesSRV = VertexBuffer->UVBufferSRV;
			LocalData.TangentsSRV = VertexBuffer->TangentBufferSRV;
			LocalData.ColorComponentsSRV = VertexBuffer->ColorBufferSRV;

			{
				LocalData.PositionComponent = FVertexStreamComponent(
					&VertexBuffer->PositionBuffer,
					0,
					sizeof(FVector3f),
					VET_Float3
				);

				LocalData.TextureCoordinates.Add(FVertexStreamComponent(
					&VertexBuffer->UVBuffer,
					0,
					sizeof(FVector2f),
					VET_Float2, 
					EVertexStreamUsage::ManualFetch
				));
			}

			#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
			SetData(RHICmdList, LocalData);		
			#else
			SetData(LocalData);
			#endif
		}

		#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		FLocalVertexFactory::InitResource(RHICmdList);
		#else
		FLocalVertexFactory::InitResource();
		#endif
	}

private:
	const FCubismDrawableVertexBuffer* VertexBuffer;
};
