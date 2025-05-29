/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Components/ActorComponent.h"
#include "CubismUpdatableInterface.h"
#include "CubismRendererComponent.generated.h"

class ACubismModel;
class ACubismMaskTexture;
class FCubismMaskJunction;
class UCubismModelComponent;

/**
 * The render order mode of the model.
 */
UENUM(BlueprintType)
enum class ECubismRendererSortingOrder : uint8
{
	FrontToBack,
	BackToFront,
};

/**
 * A component to render Live2D Cubism models.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismRendererComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:	
	/**
	 * The mask texture to apply to the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<ACubismMaskTexture> MaskTexture;

	/**
	 * The render order mode of the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismRendererSortingOrder SortingOrder = ECubismRendererSortingOrder::FrontToBack;

	/**
	 * The flag to enable the Z-sorting of the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bZSort = false;

	/**
	 * The render order of the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	int32 RenderOrder = 0;

	/**
	 * The epsilon value used for the Z-sorting of the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (EditCondition = "bZSort"))
	float Epsilon = 0.1f;

	/**
	 * The number of masks used by the model.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 NumMasks;

	/**
	 * The table that associates each Drawable with the mask it uses.
	 */
	TArray<TSharedPtr<FCubismMaskJunction>> Junctions;

	// ICubismUpdatableInterface implementation
	virtual bool IsControlledByUpdateController() const override { return true; }
	virtual int32 GetExecutionOrder() const override;
	virtual void OnCubismUpdate(float DeltaTime) override;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to apply the render order to the drawables.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 CalcRenderOrder(const UCubismDrawableComponent* Drawable) const;

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
	UCubismRendererComponent();

public:	
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
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
