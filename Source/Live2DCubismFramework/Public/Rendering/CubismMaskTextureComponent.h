/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelActor.h"
#include "Engine/TextureRenderTarget2D.h"

#include "CubismMaskTextureComponent.generated.h"

class FCubismMaskJunction;
class UCubismRendererComponent;

/**
 * A component to manage mask textures for Live2D Cubism models.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismMaskTextureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The size of the mask texture (in pixels).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	int32 Size = 4096;

	/**
	 * The flag to specify whether to use multiple render targets to manage masks.
	 * The default is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bUseMultiRenderTargets = false;

	/**
	 * The number of mask textures used if `bUseMultiRenderTargets` is `true`.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", SliderMin = "1", EditCondition = "bUseMultiRenderTargets"), Category = "Live2D Cubism")
	int32 RenderTargetCount = 1;

	/**
	 * If `bUseMultiRenderTargets` is `true`, this property specifies the level of detail of masks in each render target.
	 * The mask area of the render target is divided for each increment of this value.
	 * Therefore, the resolution of the placed masks is halved, and the number of masks placed in one render target is 4 times.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", SliderMin = "0", EditCondition = "bUseMultiRenderTargets"), Category = "Live2D Cubism")
	int32 LOD = 0;

	/**
	 * The number of masks assigned to the component.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 NumMasks;

	/**
	 * The list of ACubismModel instances whose masks are managed by the component.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<TObjectPtr<ACubismModel>> Models;

	/**
	 * The list of render targets where the assigned masks are drawn.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<TObjectPtr<UTextureRenderTarget2D>> RenderTargets;

	/**
	 * @brief The function to add a model to the component.
	 * @param Model The model to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void AddModel(ACubismModel* Model);

	/**
	 * @brief The function to remove a model from the component.
	 * @param Model The model to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void RemoveModel(ACubismModel* Model);

	/**
	 * ResolveMaskLayout
	 * @brief The function to resolve the layout of the masks.
	 * The function rearranges the masks drawn on the render target based on the specified settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void ResolveMaskLayout();

private:
	/**
	 * @brief The constructor of the component.
	 */
	UCubismMaskTextureComponent();

	/**
	 * @brief The function to calculate the optimal level of detail under the current settings.
	 * @return The optimal level of detail.
	 */
	inline int32 CalcOptimalLOD() const;

	/**
	 * @brief The function to allocate render targets.
	 * @param RequiredRTs The number of render targets to allocate.
	 */
	inline void AllocateRenderTargets(const int32 RequiredRTs);

	/**
	 * The flag to specify whether the render target needs to be updated.
	 */
	bool bDirty = true;

public:
	// UObject interface
	#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	#endif
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface
};
