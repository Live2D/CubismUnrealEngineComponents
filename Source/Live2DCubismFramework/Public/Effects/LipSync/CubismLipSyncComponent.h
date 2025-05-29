/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "CubismUpdatableInterface.h"
#include "CubismLipSyncComponent.generated.h"

class UCubismModel3Json;

/**
 * A component to apply the lip sync effect to the specified parameters of the Cubism model.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), hidecategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD))
class LIVE2DCUBISMFRAMEWORK_API UCubismLipSyncComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The json asset that contains the lip sync information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismModel3Json> Json;

	/**
	 * The value of lip sync.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism", meta=(ClampMin = "0.0", ClampMax = "1.0", SliderMin = "0.0", SliderMax = "1.0"))
	float Value = 0.0f;

	/**
	 * The mode to blend the lip sync parameter.
	 * The default is Additive.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismParameterBlendMode BlendMode = ECubismParameterBlendMode::Additive;

	/**
	 * The flag to enable the automatic lip sync.
	 * The default is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bAutoEnabled = false;

	/**
	 * The time scale of the lip sync.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition = "bAutoEnabled"), meta=(ClampMin = "0.0", ClampMax = "20.0", SliderMin = "0.0", SliderMax = "20.0"))
	float TimeScale = 10.0f;

	/**
	 * The gain to adjust the volume of the input sound.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition = "!bAutoEnabled"), meta=(ClampMin = "1.0", ClampMax = "10.0", SliderMin = "1.0", SliderMax = "10.0"))
	float Gain = 1.0f;

	/**
	 * The list of the IDs of the UCubismParameterComponent to which the parameter is applied.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<FString> Ids;

	/**
	 * The source of the input sound that the lip sync refers to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<USoundWave> Source;

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
	 * @brief The function to get the audio component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	UAudioComponent* GetAudioComponent();

protected:
	/**
	 * @brief The function to update the component.
	 * The function to update the lip sync parameter.
	 * @param DeltaTime The time interval between the previous frame and the current frame.
	 */
	void Update(const float DeltaTime);

	/**
	 * @brief The function to create the audio component.
	 * The function to create the audio component to refer to the input sound.
	 * @return The audio component to refer to the input sound.
	 */
	TObjectPtr<UAudioComponent> CreateAudioComponent();

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
	UCubismLipSyncComponent();

	/**
	 * The internal time of the component.
	 */
	float Time;

	/**
	 * The target value of the lip sync.
	 */
	float TargetValue;
	/**
	 * The smoothed current velocity of the lip sync.
	 */
	float CurrentVelocity;

	/**
	 * The audio component to refer to the input sound.
	 */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> Audio;

	/**
	 * @brief The function to return the smoothed value for the input envelope value.
	 * @param CurrentValue The current value of the lip sync.
	 * @param DeltaTime The time interval between the previous frame and the current frame.
	 */
	float SmoothDamp(const float CurrentValue, const float DeltaTime);

	/**
	 * @brief The callback function to receive the envelope value of the input sound.
	 * The function to receive the envelope value of the input sound and set the target value of the lip sync.
	 * @param InSoundWave The sound wave that the envelope value is from.
	 * @param InEnvelopeValue The envelope value of the input sound.
	 */
	UFUNCTION()
	void OnEnvelopeValue(const USoundWave* InSoundWave, const float InEnvelopeValue);

	float LipSyncTargetValue;
	float LipSyncValue;
	float LipSyncVValue;
	float LastTimeSeconds;
	float UserTimeSeconds;

public:
	// UObject interface
	void PostLoad() override;

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
