/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelComponent.h"
#include "CubismUpdatableInterface.h"
#include "CubismEyeBlinkComponent.generated.h"

class UCubismModel3Json;

/**
 * An enumeration to specify the state of eye blink.
 */
enum class ECubismEyeBlinkPhase : uint8
{
	Idle,
	Closing,
	Closed,
	Opening,
};

/**
 * A component to apply the eye blink effect to the specified parameters of the Cubism model.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismEyeBlinkComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The json asset that contains the eye blink information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismModel3Json> Json;

	/**
	 * The value of eye blink.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="!bAutoEnabled"))
	float Value = 1.0f;

	/**
	 * The mode to blend the eye blink parameter.
	 * The default is Multiplicative.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismParameterBlendMode BlendMode = ECubismParameterBlendMode::Multiplicative;

	/**
	 * The flag to enable the automatic eye blink.
	 * The default is true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bAutoEnabled = true;

	/**
	 * The mean value of the eye blink interval.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="1.0", ClampMax="10.0", SliderMin="1.0", SliderMax="10.0"))
	float Mean = 2.5f;

	/**
	 * The maximum deviation of the eye blink interval.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="0.5", ClampMax="5.0", SliderMin="0.5", SliderMax="5.0"))
	float MaximumDeviation = 2.0f;

	/**
	 * The time scale of the eye blink.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="0.0", ClampMax="20.0", SliderMin="0.0", SliderMax="20.0"))
	float TimeScale = 10.0f;

	/**
	 * The closing period of the eye blink in `Closing` phase.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="0.0", ClampMax="5.0", SliderMin="0.0", SliderMax="5.0"))
	float ClosingPeriod = 1.0f;

	/**
	 * The closed period of the eye blink in `Closed` phase.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="0.0", ClampMax="5.0", SliderMin="0.0", SliderMax="5.0"))
	float ClosedPeriod = 0.5f;

	/**
	 * The opening period of the eye blink in `Opening` phase.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta=(EditCondition="bAutoEnabled"), meta=(ClampMin="0.0", ClampMax="5.0", SliderMin="0.0", SliderMax="5.0"))
	float OpeningPeriod = 1.5f;

	/**
	 * The list of IDs of UCubismParameterComponent to which the parameter is applied.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<FString> Ids;

	// ICubismUpdatableInterface
	virtual bool IsControlledByUpdateController() const override { return true; }
	virtual int32 GetExecutionOrder() const override;
	virtual void OnCubismUpdate(float DeltaTime) override;

	/**
	 * Whether to enable blinking in editor mode.
	 */
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Live2D Cubism")
	bool bEnableEyeBlinkInEditor = false;
#endif

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
	UCubismEyeBlinkComponent();

	/**
	 * The state of eye blink.
	 */
	ECubismEyeBlinkPhase Phase;

	/**
	 * The internal time of the component.
	 */
	float Time;

	/**
	 * The start time of the current phase.
	 */
	float StartTime;

	/**
	 * @brief The function to update the state of eye blink.
	 * @param DeltaTime The time interval between the previous frame and the current frame. (milliseconds)
	 */
	void Update(const float DeltaTime);

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
