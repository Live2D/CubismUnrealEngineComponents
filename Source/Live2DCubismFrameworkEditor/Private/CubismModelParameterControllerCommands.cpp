/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismModelParameterControllerCommands.h"

#define LOCTEXT_NAMESPACE "CubismModelParameterController"

void FCubismModelParameterControllerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Cubism Model Parameter Controller", "Open the Cubism model parameter controller tab window.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
