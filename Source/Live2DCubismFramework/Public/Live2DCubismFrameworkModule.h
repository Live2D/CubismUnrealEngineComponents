/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Live2DCubismCore.h"
#include "Modules/ModuleManager.h"

class LIVE2DCUBISMFRAMEWORK_API FLive2DCubismFrameworkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	bool bIsModuleStarted;
};
