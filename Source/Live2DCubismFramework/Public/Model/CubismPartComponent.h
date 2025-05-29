/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "CubismPartComponent.generated.h"

class UCubismModelComponent;

/**
 * A component to manage a part of a Live2D Cubism model.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismPartComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The index of the part.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	int32 Index;

	/**
	 * The ID of the part.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	FString Id;

	/**
	 * The opacity of the part.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0", ClampMax = "1.0", SliderMin = "0.0", SliderMax = "1.0"))
	float Opacity = 1.0f;

	/**
	 * The flag to specify whether to overwrite the multiply color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForPartMultiplyColors;

	/**
	 * The multiply color of the part to overwrite the original model's multiply color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FLinearColor MultiplyColor = FLinearColor::White;

	/**
	 * The flag to specify whether to overwrite the screen color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForPartScreenColors;

	/**
	 * The screen color of the part to overwrite the original model's screen color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FLinearColor ScreenColor = FLinearColor::Black;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to set the opacity of the part.
	 * @param TargetOpacity The opacity to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void SetPartOpacity(float TargetOpacity);

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
	UCubismPartComponent();

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
