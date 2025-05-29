/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Expression/CubismExp3Json.h"
#include "CubismUpdatableInterface.h"
#include "CubismExpressionComponent.generated.h"

class FCubismExpression;
class UCubismModelComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCubismExpressionPlaybackFinishedHandler);

/**
 * A structure to hold the parameter value of the expression.
 */
USTRUCT(Blueprintable)
struct LIVE2DCUBISMFRAMEWORK_API FCubismExpressionParameterValue
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The index of the parameter to update.
	 */
	int32 Index;

	/**
	 * The ID of the UCubismParameterComponent to update.
	 */
	FString Id;

	/**
	 * The additive value used to update the parameter.
	 */
	float AdditiveValue;

	/**
	 * The multiplier value used to update the parameter.
	 */
	float MultiplyValue;

	/**
	 * The overwriting value used to update the parameter.
	 */
	float OverwriteValue;
};

/**
 * A component to apply the expression motion to the specified parameters of the Cubism model.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismExpressionComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The index of the expression to play.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "-1", SliderMin = "-1"))
	int32 Index = -1;

	/**
	 * The json assets that contain the expression information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<TObjectPtr<UCubismExp3Json>> Jsons;

	/**
	 * The delegate to be called when the expression motion playback is finished.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Live2D Cubism")
	FCubismExpressionPlaybackFinishedHandler OnExpressionPlaybackFinished;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to play the expression motion.
	 * @param InIndex The index of the expression to play.
	 * @param InPriority The priority of the expression.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void PlayExpression(const int32 InIndex);

	/**
	 * @brief The function to stop the expression motion.
	 * @param bForce The flag to force to stop the expression motion.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void StopAllExpressions(const bool bForce = false);

	// ICubismUpdatableInterface
	virtual bool IsControlledByUpdateController() const override { return true; }
	virtual int32 GetExecutionOrder() const override;
	virtual void OnCubismUpdate(float DeltaTime) override;

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
	UCubismExpressionComponent();

	/**
	 * The internal time of the component.
	 */
	float Time;

	/**
	 * The queue of the expression motion to play.
	 */
	TArray<TSharedPtr<FCubismExpression>> ExpressionQueue;

	/**
	 * The list of the parameter values of the expression.
	 */
	TArray<FCubismExpressionParameterValue> ParameterValues;

	/**
	 * @brief The function to update the parameters of the expression.
	 * @param ExpressionIndex The index of the expression in the queue.
	 * @param Expression The expression motion state to update.
	 */
	void UpdateExpression(const int32 ExpressionIndex, const TSharedPtr<FCubismExpression>& Expression);

	/**
	 * @brief The function to performs blend calculations with the entered values.
	 * @param Source The current fade value.
	 * @param Destination The applied fade value.
	 * @param FadeWeight The fade weight of the expression.
	 */
	static float CalculateValue(float Source, float Destination, float FadeWeight);

public:
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
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
