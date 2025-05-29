/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"

#include "CubismUserData3Json.generated.h"

/**
 * An enumeration to specify the target of user data.
 */
UENUM(BlueprintType)
enum class ECubismUserDataTargetType : uint8
{
	ArtMesh,
};

/**
 * A structure corresponding to an array element of the `UserData` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismUserDataEntry
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` and `Value` fields in a user data object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "User Data")
	TMap<FString, FString> Tags;
};

/**
 * A structure to hold the user data read from userdata3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismUserData3Json : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The property corresponding to the `UserDataCount` field in `Meta` field.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "User Data")
	int32 Size;

	/**
	 * The property corresponding to the `UserData` field in a user data object.
	 * The key corresponds to the `Target` field in a user data object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "User Data")
	TMap<ECubismUserDataTargetType, FCubismUserDataEntry> Data;

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
