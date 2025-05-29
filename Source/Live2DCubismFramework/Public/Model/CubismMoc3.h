/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Live2DCubismCore.h"
#include "EditorFramework/AssetImportData.h"

#include "CubismMoc3.generated.h"

typedef void (*CubismLogFunction)(const char* message);

/**
 * A class to wrap a Live2D Cubism moc.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismMoc3 : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The version of the moc.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Moc Data")
	int32 Version;

public:
	/**
	 * @brief The function to load the moc data into memory and assign it to the model.
	 * @param Model The model to assign the moc to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	void SetupModel(UCubismModelComponent* InModel);

	/**
	 * @brief The function to delete the moc data assigned to the model and free the memory.
	 * @param Model The model to delete the moc data from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	void DeleteModel(UCubismModelComponent* InModel);

	/**
	 * @brief The function to set up the moc data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	void Setup();

	////

	/**
	 * @brief The function to get the version of the Cubism Core library.
	 * @return The version of the Cubism Core library.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	static int32 GetVersion();

	/**
	 * @brief The function to get the latest version of the moc that the Cubism Core library supports.
	 * @return The latest version of the moc that the Cubism Core library supports.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	static int32 GetLatestMocVersion();

	/**
	 * @brief The function to get the version of the moc.
	 * @param Address The top address of the moc data. 
	 * @param Size The byte size of the moc data.
	 * @return The version of the moc.
	 */
	static int32 GetMocVersion(const void* Address, const int32 Size);

	/**
	 * @brief The function to check the consistency of the moc data with the Cubism Core library.
	 * @param Address The top address of the moc data.
	 * @param Size The byte size of the moc data.
	 * @return True if the moc data has consistency with the Cubism Core library, false otherwise.
	 */
	static bool HasMocConsistency(void* Address, const int32 Size);

	////

	/**
	 * @brief The function to get the log function.
	 * @return The log function.
	 */
	CubismLogFunction GetLogFunction() const;

	/**
	 * @brief The function to set the log function.
	 * @param LogFunction The log function.
	 */
	void SetLogFunction(CubismLogFunction LogFunction);

	////

	/**
	 * @brief The function to get the byte size of the model.
	 * @return The byte size of the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Moc Data")
	int32 GetSizeOfModel() const;

private:
	friend class UCubismMoc3Factory;

	/**
	 * The byte array of the moc3
	 */
	UPROPERTY(SaveGame)
	TArray<uint8> Bytes;

private:
	/**
	 * The raw moc data 
	 */
	csmMoc* RawMoc;

public:
	// UObject interface
	virtual void PostLoad() override;
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
