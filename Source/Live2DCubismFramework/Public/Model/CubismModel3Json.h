/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"

#include "CubismModel3Json.generated.h"

/**
 * A structure corresponding to an array element of the `Expressions` field in a model3.json file.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FExpressionEntry
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Name` field in an expression object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString Name;

	/**
	 * The property corresponding to the `File` field in an expression object.
	 * This field specifies the path to the exp3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString Path;
};

/**
 * A structure corresponding to an array element of the `Motions` field in a model3.json file.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FMotionGroupEntry
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the key field in a motion group object.
	 * This field specifies the name of the motion group.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString Name;

	/**
	 * The property corresponding to the value field in a motion group object.
	 * This field specifies the paths to the motion3.json files.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FString> Paths;
};

/**
 * A structure corresponding to an array element of the `HitAreas` field in a model3.json file.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FHitAreaEntry
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a hit area object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString Id;

	/**
	 * The property corresponding to the `Name` field in a hit area object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString Name;
};

/**
 * A structure corresponding to the model data read from a model3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismModel3Json : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * The property corresponding to the `Version` field in a model3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	int32 Version;

	/**
	 * The property corresponding to the `Moc` field in a model3.json file.
	 * This field specifies the path to the moc3 file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString MocPath;

	/**
	 * The property corresponding to the `Textures` field in a model3.json file.
	 * This field specifies the paths to the texture files.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FString> TexturePaths;

	/**
	 * The property corresponding to the `Physics` field in a model3.json file.
	 * This field specifies the path to the physics3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString PhysicsPath;

	/**
	 * The property corresponding to the `Pose` field in a model3.json file.
	 * This field specifies the path to the pose3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString PosePath;

	/**
	 * The property corresponding to the `DisplayInfo` field in a model3.json file.
	 * This field specifies the path to the cdi3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString DisplayInfoPath;

	/**
	 * The property corresponding to the `Expressions` field in a model3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FExpressionEntry> Expressions;

	/**
	 * The property corresponding to the `Motions` field in a model3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FMotionGroupEntry> Motions;

	/**
	 * The property corresponding to the `UserData` field in a model3.json file.
	 * This field specifies the path to the userdata3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	FString UserDataPath;

	/**
	 * The property to specify the parameters to which the EyeBlink effect is applied.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FString> EyeBlinks;

	/**
	 * The property to specify the parameters to which the LipSync effect is applied.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FString> LipSyncs;

	/**
	 * The property corresponding to the `HitAreas` field in a model3.json file.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Data")
	TArray<FHitAreaEntry> HitAreas;

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
