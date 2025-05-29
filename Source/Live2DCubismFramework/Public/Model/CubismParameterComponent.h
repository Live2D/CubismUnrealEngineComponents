/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "CubismParameterComponent.generated.h"

/**
 * A component to manage a parameter of a Live2D Cubism model.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismParameterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The index of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 Index;

	/**
	 * The ID of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	FString Id;

	/**
	 * The type of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	ECubismParameterType Type;

	/**
	 * The minimum value of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	float MinimumValue;

	/**
	 * The maximum value of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	float MaximumValue;

	/**
	 * The default value of the parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	float DefaultValue;

	/**
	 * The value of the parameter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	float Value;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to set the value of the parameter.
	 * @param TargetValue The value to set.
	 * @param Weight The [0,1] weight to specify interpolation between the current value and `TargetValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void SetParameterValue(float TargetValue, const float Weight = 1.0f);

	/**
	 * @brief The function to add the value to the parameter.
	 * @param TargetValue The value to add.
	 * @param Weight The [0,1] weight to specify interpolation between 0 and `TargetValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void AddParameterValue(float TargetValue, const float Weight = 1.0f);

	/**
	 * @brief The function to multiply the value to the parameter.
	 * @param TargetValue The value to multiply.
	 * @param Weight The [0,1] weight to specify interpolation between 1 and `TargetValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void MultiplyParameterValue(float TargetValue, const float Weight = 1.0f);

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
	UCubismParameterComponent();

public:
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
	// End of UActorComponent interface
};
