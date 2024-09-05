/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Physics/CubismPhysicsRig.h"

#include "CubismPhysicsComponent.generated.h"

class UCubismModelComponent;

/**
 * A component to apply the physics to the specified parameters of the Cubism model.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The json assets that contain the physics information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismPhysics3Json> Json;

	/**
	 * The gravity vector to apply to the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FVector2D Gravity = FVector2D(0.0f, -1.0f);

	/**
	 * The wind vector to apply to the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FVector2D Wind = FVector2D::ZeroVector;

	/**
	 * The update frequency of the physics calculation (FPS).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "120.0"))
	float Fps = 0.0f;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to stabilize the physics calculation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Stabilization();

private:
	/**
	 * @brief The constructor of the component.
	 */
	UCubismPhysicsComponent();

	/**
	 * @brief The model component that the component depends on.
	 */
	TObjectPtr<UCubismModelComponent> Model;

	/**
	 * The physics state of the model.
	 */
	TArray<FCubismPhysicsRig> Rigs;

	/**
	 * The time to remain until the next physics calculation.
	 */
	float CurrentRemainTime;

	TArray<float> ParameterCaches;
	TArray<float> ParameterInputCaches;

	/**
	 * The function to initialize the physics state of the model.
	 */
	void Initialize();

	/**
	 * The function to update the physics state of the model.
	 * @param Particles The particles that consist of the rig.
	 * @param TotalTranslation The translation of the root particle.
	 * @param TotalAngle The angle of the root particle.
	 * @param ThresholdValue The threshold value to determine whether to update the physics state.
	 * @param DeltaTime The time interval between the previous frame and the current frame.
	 * @param Resistance The resistance value to apply to the verteces.
	 */
	void UpdateParticles(
		TArray<FCubismPhysicsRigParticle>& Particles,
		const FVector2D TotalTranslation,
		const float TotalAngle,
		const float ThresholdValue,
		const float DeltaTime,
		const float Resistance
	);

	/**
	 * The function to update the physics state of the model for stabilization.
	 * @param Particles The particles that consist of the rig.
	 * @param TotalTranslation The translation of the root particle.
	 * @param TotalAngle The angle of the root particle.
	 * @param ThresholdValue The threshold value to determine whether to update the physics state.
	 */
	void UpdateParticlesForStabilization(
		TArray<FCubismPhysicsRigParticle>& Particles,
		const FVector2D TotalTranslation,
		const float TotalAngle,
		const float ThresholdValue
	);

public:
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface
};
