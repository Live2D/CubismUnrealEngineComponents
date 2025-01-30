/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Framework/Commands/Commands.h"

class FCubismModelParameterControllerCommands : public TCommands<FCubismModelParameterControllerCommands>
{
public:

	FCubismModelParameterControllerCommands()
		: TCommands<FCubismModelParameterControllerCommands>(
			TEXT("CubismModelParameterController"),
			NSLOCTEXT("Contexts", "CubismModelParameterController", "Cubism Model Parameter Controller"),
			NAME_None,
			FName(TEXT("CubismModelParameterControllerStyle"))
		)
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
