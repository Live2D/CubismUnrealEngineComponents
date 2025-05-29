/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include "Components/ActorComponent.h"
#include "CubismUpdateControllerComponent.generated.h"

class ICubismUpdatableInterface;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LIVE2DCUBISMFRAMEWORK_API UCubismUpdateControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCubismUpdateControllerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnComponentCreated() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RefreshUpdatables();

private:
	UPROPERTY()
	TArray<TScriptInterface<ICubismUpdatableInterface>> Updatables;
};