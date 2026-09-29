/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once


/**
 * Index buffer for a drawable.
 */
class FCubismUshortIndexBuffer : public FIndexBuffer
{
public:
	static constexpr uint32 Stride = sizeof(uint16);

	virtual ~FCubismUshortIndexBuffer() override
	{
		delete Indices;
	}

	void Init(const TArray<uint16>& InIndices, const FString& InResourceName, bool InIsNeedsCpuAccess)
	{
		NumIndices = InIndices.Num();

		delete Indices;
		Indices = new TResourceArray<uint16>(InIsNeedsCpuAccess);
		Indices->Append(InIndices);
		Data = Indices->GetData();

		ResourceName = InResourceName;
	}

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCubismDrawableIndexBuffer::InitRHI)

		const uint32 SizeInBytes = Stride * NumIndices;

		FRHIResourceCreateInfo IndexBufferInfo(*ResourceName);
		IndexBufferInfo.ResourceArray = Indices;
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(Stride, SizeInBytes, BUF_Dynamic, IndexBufferInfo);

	}

	virtual void ReleaseRHI() override
	{
		IndexBufferRHI.SafeRelease();
		FIndexBuffer::ReleaseRHI();
	}

	void* GetIndexData()
	{
		return Data;
	}

	const void* GetIndexData() const
	{
		return Data;
	}

	uint32 GetSizeBytes() const
	{
		return NumIndices * Stride;
	}

	uint32 GetNumIndices() const
	{
		return NumIndices;
	}

private:
	FString ResourceName = {};

	uint32 NumIndices = 0U;
	TResourceArray<uint16>* Indices = nullptr;

	void* Data = nullptr;
};
