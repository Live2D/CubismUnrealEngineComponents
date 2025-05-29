/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Components/ActorComponent.h"
#include "CubismLookAtComponent.generated.h"

class UCubismModelComponent;
struct FCubismLookAtParameter;

/**
 * A component to apply the look-at effect to the specified parameters of the Cubism model.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LIVE2DCUBISMFRAMEWORK_API UCubismLookAtComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The target actor to look at.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<AActor> Target;

	/**
	 * The parameters to control the look-at effect.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<FCubismLookAtParameter> Parameters;

	/**
	 * The smoothing value of the look-at effect.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0001", ClampMax = "1.0", SliderMin = "0.0001", SliderMax = "1.0"))
	float Smoothing = 0.15f;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

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
	UCubismLookAtComponent();

	/**
	 * The smoothed position of the look-at target.
	 */
	FVector LastPosition;
	/**
	 * The smoothed current velocity of the look-at target.
	 */
	FVector CurrentVelocity;

	/**
	 * @brief The function to return the smoothed value for the look-at target position.
	 * @param CurrentValue The current value of the look-at target position.
	 * @param DeltaTime The time interval between the previous frame and the current frame.
	 */
	FVector SmoothDamp(const FVector CurrentValue, const float DeltaTime);

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
