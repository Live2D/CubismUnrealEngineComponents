/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Motion/CubismMotion3Json.h"
#include "CubismUpdatableInterface.h"
#include "Components/ActorComponent.h"

#include "CubismMotionComponent.generated.h"

class UCubismModelComponent;
class FCubismMotion;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCubismMotionPlaybackFinishedHandler);

/**
 * An enumeration of the priority of the motion.
 */
UENUM()
enum class ECubismMotionPriority : uint8
{
	None,
	Idle,
	Normal,
	Force,
};

/**
 * A component to apply the motion to the specified parameters of the Cubism model.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismMotionComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The index of the motion to play.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	int32 Index = -1;

	/**
	 * The json assets that contain the motion information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<TObjectPtr<UCubismMotion3Json>> Jsons;

	/**
	 * The playback speed of the motion.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0", SliderMin = "0.0", SliderMax = "10.0"))
	float Speed = 1.0f;

	/**
	 * The current priority of the motion.
	 * If no motion is playing, the priority is None.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	ECubismMotionPriority CurrentPriority = ECubismMotionPriority::None;

	/**
	 * The reserved priority of the motion.
	 * If no motion is reserved, the priority is None.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	ECubismMotionPriority ReservedPriority = ECubismMotionPriority::None;

	/**
	 * The delegate to be called when the motion playback is finished.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Live2D Cubism")
	FCubismMotionPlaybackFinishedHandler OnMotionPlaybackFinished;

	// ICubismUpdatableInterface
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
	 * @brief The function to check if the all motions are finished.
	 * @return True if the all motions are finished, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	bool IsFinished() const;

	/**
	 * @brief The function to reserve the motion.
	 * @param Priority The priority of the motion.
	 * @return True if the motion is reserved, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	bool ReserveMotion(const ECubismMotionPriority Priority);

	/**
	 * @brief The function to play the motion.
	 * @param Json The json asset that contains the motion information.
	 * @param OffsetTime The delay time from the start of the motion to the start of the fade-in.
	 * @param Priority The priority of the motion.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void PlayMotion(const int32 InIndex, const float OffsetTime=0.0f, const ECubismMotionPriority Priority=ECubismMotionPriority::Normal);

	/**
	 * @brief The function to stop the motion.
	 * @param bForce True if the motion is forced to stop, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void StopAllMotions(const bool bForce = false);

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
	UCubismMotionComponent();

	/**
	 * The internal time of the component.
	 */
	float Time;

	/**
	 * The queue of the motion to play.
	 */
	TArray<TSharedPtr<FCubismMotion>> MotionQueue;

	/**
	 * @brief The function to update the motion.
	 * @param Motion The motion to update.
	 */
	void UpdateMotion(float UserTimeSeconds, float FadeWeight, const TSharedPtr<FCubismMotion>& CubismMotion);

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
