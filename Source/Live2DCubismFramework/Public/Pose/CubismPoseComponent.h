/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Components/ActorComponent.h"
#include "CubismUpdatableInterface.h"
#include "CubismPoseComponent.generated.h"

class UCubismModelComponent;
class UCubismParameterComponent;
class UCubismPartComponent;
class UCubismPose3Json;

/**
 * A structure to hold the list of the part and the parts that are linked to the part.
 */
struct FCubismPosePartParameter
{
	/**
	 * The part that the pose is applied to.
	 */
	TObjectPtr<UCubismPartComponent> Part;

	/**
	 * The parameter that the pose is reffered to.
	 */
	TObjectPtr<UCubismParameterComponent> Parameter;

	/**
	 * The list of the parts that are linked to the part. 
	 */
	TArray<TObjectPtr<UCubismPartComponent>> LinkParts;
};

/**
 * A structure to hold the group of the exclusive parts.
 */
struct FCubismPosePartGroupParameter
{
	/**
	 * the group of the exclusive parts.
	 */
	TArray<FCubismPosePartParameter> Parts;
};

/**
 * A component to apply the pose to the specified parameters of the Cubism model.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, ImplementsInterface = "CubismUpdatableInterface"))
class LIVE2DCUBISMFRAMEWORK_API UCubismPoseComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The time to fade in.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.01", ClampMax = "10.0", SliderMin = "0.0", SliderMax = "10.0"))
	float FadeInTime;

	/**
	 * The json assets that contain the pose information.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismPose3Json> Json;

	// CubismUpdatableInterface implementation
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
	UCubismPoseComponent();

	/**
	 * @brief Perform the fade operation on the part.
	 *
	 * @param[in]   DeltaTime            Delta time [seconds]
	 */
	void DoFade(float DeltaTime);

	/**
	 * @brief Copy the opacity of the part.
	 *
	 * Copy the opacity of the part and set it on the linked parts.
	 *
	 */
	void CopyPartOpacities();

	/**
	 * The list of the parts.
	 */
	TArray<FCubismPosePartGroupParameter> PartGroups;

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

	/**
	 *Whether to enable pose updates in editor mode.
	 */
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Live2D Cubism")
	bool bEnablePoseInEditor = false;
#endif
};
