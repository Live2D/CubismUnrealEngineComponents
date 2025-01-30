/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "EditorFramework/AssetImportData.h"

#include "CubismExp3Json.generated.h"

/**
 * A structure corresponding to an array element of the `Parameters` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismExpressionParameter
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString Id;

	/**
	 * The property corresponding to the `Value` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	float Value;

	/**
	 * The property corresponding to the `Blend` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	ECubismParameterBlendMode Blend;

	FCubismExpressionParameter()
		: Id()
		, Value(0.0f)
		, Blend(ECubismParameterBlendMode::Additive)
	{ }
};

/**
 * A class to hold the expression data read from exp3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismExp3Json : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The property corresponding to the `Type` field in an expression object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression Data")
	FString Type;

	/**
	 * The property corresponding to the `FadeInTime` field in an expression object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression Data")
	float FadeInTime;

	/**
	 * The property corresponding to the `FadeOutTime` field in an expression object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression Data")
	float FadeOutTime;

	/**
	 * The property corresponding to the `Parameters` field in an expression object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression Data")
	TArray<FCubismExpressionParameter> Parameters;

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
