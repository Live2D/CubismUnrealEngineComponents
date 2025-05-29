/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Components/ActorComponent.h"

#include "CubismParameterStoreComponent.generated.h"

class UCubismModelComponent;

/**
 * A component to store the parameters of a Live2D Cubism model.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismParameterStoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to save the value of a parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void SaveParameterValue(const int32 ParameterIndex);

	/**
	 * @brief The function to save the opacity of a part at the specified index.
	 * @param PartIndex The index of the part.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void SavePartOpacity(const int32 PartIndex);

	/**
	 * @brief The function to save the parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void SaveParameters();

	/**
	 * @brief The function to load the parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void LoadParameters();

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
	UCubismParameterStoreComponent();

	/**
	 * The saved values of the parameters.
	 */
	TMap<int32, float> ParameterValues;

	/**
	 * The saved opacities of the parts.
	 */
	TMap<int32, float> PartOpacities;

public:
	// UObject interface
	virtual void PostLoad() override;
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
