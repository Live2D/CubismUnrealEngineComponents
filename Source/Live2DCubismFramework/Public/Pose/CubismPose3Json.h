/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"

#include "CubismPose3Json.generated.h"

/**
 * A structure corresponding to an array element of a pose part object.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPosePart
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a pose part object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	FString Id;

	/**
	 * The property corresponding to the `Link` field in a pose part object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	TArray<FString> Links;
};

/**
 * A structure corresponding to an array element of the `Groups` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPosePartGroup
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to an array element of a pose part group object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part Group")
	TArray<FCubismPosePart> Parts;
};

/**
 * A structure to hold the pose data read from pose3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismPose3Json : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The property corresponding to the `Version` field in a pose object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose Data")
	int32 Version;

	/**
	 * The property corresponding to the `FadeInTime` field in a pose object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose Data")
	float FadeInTime;

	/**
	 * The property corresponding to the `Groups` field in a pose object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose Data")
	TArray<FCubismPosePartGroup> PartGroups;

	// UObject interface
	virtual void PostInitProperties() override;
	// End of UObject interface

#if WITH_EDITORONLY_DATA
	// Import data for this 
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSettings)
	TObjectPtr<class UAssetImportData> AssetImportData;

	// UObject interface
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface
#endif
};
