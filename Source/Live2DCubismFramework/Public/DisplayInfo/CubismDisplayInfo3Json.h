/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"

#include "CubismDisplayInfo3Json.generated.h"

/**
 * A structure corresponding to an array element of the `Parameters` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismDisplayInfoParameter
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString Id;

	/**
	 * The property corresponding to the `GroupId` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString GroupId;

	/**
	 * The property corresponding to the `Name` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	FText Name;
};

/**
 * A structure corresponding to an array element of the `ParameterGroups` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismDisplayInfoParameterGroup
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a parameter group object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParameterGroup")
	FString Id;

	/**
	 * The property corresponding to the `GroupId` field in a parameter group object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParameterGroup")
	FString GroupId;

	/**
	 * The property corresponding to the `Name` field in a parameter group object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParameterGroup")
	FText Name;
};

/**
 * A structure corresponding to an array element of the `Parts` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismDisplayInfoPart
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a part object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	FString Id;

	/**
	 * The property corresponding to the `Name` field in a part object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	FText Name;
};

/**
 * A structure to hold the display info data read from cdi3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismDisplayInfo3Json : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The property corresponding to the `Parameters` field in cdi3.json.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TArray<FCubismDisplayInfoParameter> Parameters;

	/**
	 * The property corresponding to the `ParameterGroups` field in cdi3.json.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TArray<FCubismDisplayInfoParameterGroup> ParameterGroups;

	/**
	 * The property corresponding to the `Parts` field in cdi3.json.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info Data")
	TArray<FCubismDisplayInfoPart> Parts;

	virtual void PostInitProperties() override;

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
