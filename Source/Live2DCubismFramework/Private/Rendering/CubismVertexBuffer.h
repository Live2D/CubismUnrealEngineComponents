/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once


template<typename ElementType>
class FCubismVertexBuffer : public FVertexBuffer
{
public:
	static constexpr uint32 Stride = sizeof(ElementType);

	virtual ~FCubismVertexBuffer() override
	{
		delete VertexData;
	}

	void Init(const TArray<ElementType>& InPositions, const FString& InResourceName, bool InIsNeedsCpuAccess)
	{
		NumVertices = InPositions.Num();

		delete VertexData;
		VertexData = new TStaticMeshVertexData<ElementType>(InIsNeedsCpuAccess);
		VertexData->ResizeBuffer(NumVertices);

		Data = VertexData->GetDataPointer();
		FMemory::Memcpy(Data, InPositions.GetData(), GetSizeBytes());

		ResourceName = InResourceName;
	}

	void Init(const ElementType& InInitialValue, const uint32 InNumVertices, const FString& InResourceName, bool InIsNeedsCpuAccess)
	{
		TArray<ElementType> Positions;
		Positions.Init(InInitialValue, InNumVertices);
		Init(Positions, InResourceName, InIsNeedsCpuAccess);
	}

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		VertexBufferRHI = FRenderResource::CreateRHIBuffer(RHICmdList, VertexData, NumVertices, BUF_Dynamic | BUF_ShaderResource, *ResourceName);
#else
		VertexBufferRHI = FRenderResource::CreateRHIBuffer<true>(VertexData, NumVertices, BUF_Dynamic | BUF_ShaderResource, *ResourceName);
#endif
		if (VertexBufferRHI)
		{
			ComponentSRV = RHICmdList.CreateShaderResourceView(VertexBufferRHI, 8, PF_G32R32F);
		}
	}

	virtual void ReleaseRHI() override
	{
		ComponentSRV.SafeRelease();
		FVertexBuffer::ReleaseRHI();
	}

	uint32 GetSizeBytes() const
	{
		return NumVertices * Stride;
	}

	uint32 GetNumVertices() const
	{
		return NumVertices;
	}

	FRHIShaderResourceView* GetSRV() const
	{
		return ComponentSRV;
	}

	void* GetVertexData()
	{
		return Data;
	}

	const void* GetVertexData() const
	{
		return Data;
	}

private:
	FString ResourceName = {};

	uint32 NumVertices = 0U;
	FStaticMeshVertexDataInterface* VertexData = nullptr;
	uint8* Data = nullptr;

	FShaderResourceViewRHIRef ComponentSRV = {};
};
