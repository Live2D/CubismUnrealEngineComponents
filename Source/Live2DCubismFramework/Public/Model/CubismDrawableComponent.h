/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "Components/MeshComponent.h"
#include "CubismDrawableComponent.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

/**
 * A component to manage a drawable in a Live2D Cubism model.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismDrawableComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	/**
	 * The index of the drawable.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 Index;

	/**
	 * The ID of the drawable.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	FString Id;

	/**
	 * The order of the drawable for rendering from original model data.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 RenderOrder;

	/**
	 * The custom offset value of the rendering order.
	 * The value is added to the original rendering order.
	 * The default value is 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	int32 RenderOrderOffset = 0;

	/**
	 * The index of the texture assigned to the drawable.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 TextureIndex;

	/**
	 *  The opacity of the drawable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0", ClampMax = "1.0", SliderMin = "0.0", SliderMax = "1.0"))
	float Opacity;

	/**
	 * The base color of the drawable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FLinearColor BaseColor;

	/**
	 * The flag to specify whether to overwrite the multiply color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForDrawableMultiplyColors;

	/**
	 * The multiply color of the drawable to overwrite the original model's multiply color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (EditCondition = "bOverwriteFlagForDrawableMultiplyColors"))
	FLinearColor MultiplyColor;

	/**
	 * The flag to specify whether to overwrite the screen color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForDrawableScreenColors;

	/**
	 * The screen color of the drawable to overwrite the original model's screen color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (EditCondition = "bOverwriteFlagForDrawableScreenColors"))
	FLinearColor ScreenColor;

	/**
	 * The flag to specify whether to overwrite the culling set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForDrawableIsTwoSided;

	/**
	 * The flag to specify whether the drawable is two-sided.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Live2D Cubism", meta = (EditCondition = "bOverwriteFlagForDrawableIsTwoSided"))
	bool bTwoSided;

	/**
	 * The index of the parent part.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 ParentPartIndex;

	/**
	 * The blend mode of the drawable.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	ECubismDrawableBlendMode BlendMode;

	/**
	 * The flag to specify whether to invert the mask.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	bool InvertedMask;

	/**
	 * The list of indices of the drawables for masking.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<int32> Masks;

	/**
	 * The tag assigned to the drawable by UCubismUserDataComponent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FString UserDataTag;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to check whether the drawable is masked.
	 * @return True if the drawable is masked, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	inline bool IsMasked() const
	{
		return Masks.Num() > 0;
	}

	/**
	 * @brief The function to convert the vertex position from the local(model) space to the global space.
	 * @param VertexPosition The vertex position in the local(model) space.
	 * @return The vertex position in the global space.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FVector ToGlobalPosition(const FVector2D VertexPosition) const;

	/**
	 * @brief The function to get the vertex indices of the drawable.
	 * @return The vertex indices of the drawable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	TArray<int32> GetVertexIndices() const;

	/**
	 * @brief The function to get the vertex positions of the drawable.
	 * @return The vertex positions of the drawable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	TArray<FVector2D> GetVertexPositions() const;

	/**
	 * @brief The function to get the vertex Uvs of the drawable.
	 * @return The vertex Uvs of the drawable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	TArray<FVector2D> GetVertexUvs() const;

	/**
	 * @brief The function to get the indices of the drawables for masking
	 * @return The list of the indices of the drawables for masking.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	const TArray<int32> GetDrawableMask() const;

	/**
	 * @brief The function to get the number of the indices of the drawables for masking
	 * @return The number of the drawables for masking.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetDrawableMaskCount() const;

private:
	friend class UCubismModelComponent;

	/**
	 * The model component that the component depends on.
	 */
	UPROPERTY()
	TObjectPtr<UCubismModelComponent> Model;

private:
	/**
	 * @brief The constructor of the component.
	 */
	UCubismDrawableComponent();

	/**
	 * The flag to indicate whether the rectangle surrounding the vertices that consist of the drawable needs to be updated.
	 */
	mutable bool bBoundsDirty;

	/**
	 * The rectangle surrounding the vertices that consist of the drawable.
	 */
	mutable FBoxSphereBounds LocalBounds;

	/**
	 * The vertex indices of the drawable.
	 */
	UPROPERTY()
	TArray<int32> VertexIndices;

	/**
	 * The vertex positions of the drawable.
	 */
	UPROPERTY()
	TArray<FVector2D> VertexPositions;

	/**
	 * The vertex Uvs of the drawable.
	 */
	UPROPERTY()
	TArray<FVector2D> VertexUvs;

	/**
	 * The cache of the multiply color for overwriting.
	 */
	FLinearColor UserMultiplyColor;

	/**
	 * The cache of the screen color for overwriting.
	 */
	FLinearColor UserScreenColor;

	/**
	 * The cache of the flag to specify whether the drawable is two-sided.
	 */
	bool bUserTwoSided;

public:
	//Begin USceneComponent Interface
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//End USceneComponent Interface

private:
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	virtual void SendRenderDynamicData_Concurrent() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface

	//Begin UMeshComponent Interface
	virtual int32 GetNumMaterials() const override { return 1; }
	//End UMeshComponent Interface

	//Begin UPrimitiveComponent Interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//End UPrimitiveComponent Interface
};
