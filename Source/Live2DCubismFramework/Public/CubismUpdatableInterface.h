/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "UObject/Interface.h"
#include "CubismUpdatableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCubismUpdatableInterface : public UInterface
{
	GENERATED_BODY()
};

class ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	// Update priority (lower = earlier)
	virtual int32 GetExecutionOrder() const = 0;

	virtual void OnCubismUpdate(float DeltaTime) = 0;

	// If true, will not update via Tick
	virtual bool IsControlledByUpdateController() const = 0;
};