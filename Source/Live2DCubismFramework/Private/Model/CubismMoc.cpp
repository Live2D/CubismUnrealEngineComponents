/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismMoc3.h"

#include "Model/CubismModelComponent.h"
#include "CubismLog.h"

void UCubismMoc3::SetupModel(UCubismModelComponent* InModel)
{
	const uint32 Size = csmGetSizeofModel(RawMoc);

	void* ModelAddress = FMemory::Malloc(Size, csmAlignofModel);

	csmModel* RawModel = csmInitializeModelInPlace(RawMoc, ModelAddress, Size);

	UE_LOG(LogCubism, Log, TEXT("Cubism Model Has Created!: %016x"), RawModel);

	if (InModel->RawModel)
	{
		DeleteModel(InModel);
	}

	InModel->Moc = this;
	InModel->RawModel = RawModel;
}

void UCubismMoc3::DeleteModel(UCubismModelComponent* InModel)
{
	InModel->Moc = nullptr;

	FMemory::Free(InModel->RawModel);

	UE_LOG(LogCubism, Log, TEXT("Cubism Model Has Destroyed!: %016x"), InModel->RawModel);

	InModel->RawModel = nullptr;
}

void UCubismMoc3::Setup()
{
	const uint32 Size = Bytes.Num();

	void* MocAddress = FMemory::Malloc(Size, csmAlignofMoc);

	FMemory::Memcpy(MocAddress, Bytes.GetData(), Size);

	RawMoc = csmReviveMocInPlace(MocAddress, Size);

	Version = GetMocVersion(MocAddress, Size);
	if (Version < GetLatestMocVersion())
	{
		UE_LOG(LogCubism, Warning, TEXT("Moc Version is outdated. Moc Version: %d, Latest Moc Version: %d") , Version, GetLatestMocVersion());
	}
	else
	{
		UE_LOG(LogCubism, Log, TEXT("Moc Version: %d"), Version);
	}
}

int32 UCubismMoc3::GetVersion()
{
	return csmGetVersion();
}

int32 UCubismMoc3::GetLatestMocVersion()
{
	return csmGetLatestMocVersion();
}

int32 UCubismMoc3::GetMocVersion(const void* Address, const int32 Size)
{
	return csmGetMocVersion(Address, Size);
}

bool UCubismMoc3::HasMocConsistency(void* Address, const int32 Size)
{
	return csmHasMocConsistency(Address, Size) != 0;
}

////

CubismLogFunction UCubismMoc3::GetLogFunction() const
{
	return csmGetLogFunction();
}

////

void UCubismMoc3::SetLogFunction(CubismLogFunction LogFunction)
{
	csmSetLogFunction(LogFunction);
}

////

int32 UCubismMoc3::GetSizeOfModel() const
{
	return csmGetSizeofModel(RawMoc);
}

void UCubismMoc3::PostLoad()
{
	Super::PostLoad();

	Setup();
}

void UCubismMoc3::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif
	Super::PostInitProperties();
}

#if WITH_EDITORONLY_DATA
void UCubismMoc3::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add( FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden) );
	}

	Super::GetAssetRegistryTags(OutTags);
}
void UCubismMoc3::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.UEVer() < VER_UE4_ASSET_IMPORT_DATA_AS_JSON && !AssetImportData)
	{
		// AssetImportData should always be valid
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
}
#endif
