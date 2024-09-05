/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismUserDataComponent.generated.h"

class UCubismModelComponent;
class UCubismUserData3Json;

/**
 * A component to manage user data for Live2D Cubism models.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismUserDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * The json asset that contains the user data.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismUserData3Json> Json;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

private:
	/**
	 * The model component that the component depends on.
	 */
	TObjectPtr<UCubismModelComponent> Model;

public:
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;
	// End of UActorComponent interface
};
