/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "UserData/CubismUserData3Json.h"

/**
 * A support class for importing userdata3.json files.
 */
class FCubismUserData3JsonImporter
{
public:
	bool ImportFromString(const FString& FileContent);

	void ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismUserData3Json>& Json);

private:
	static TSharedPtr<FJsonObject> ParseJSON(const FString& FileContent, bool bSilent);

protected:
	int32 Version;

	int32 Size;

	TMap<ECubismUserDataTargetType, FCubismUserDataEntry> Data;
};
