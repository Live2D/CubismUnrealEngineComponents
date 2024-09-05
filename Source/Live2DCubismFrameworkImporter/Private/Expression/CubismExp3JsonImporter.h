/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Expression/CubismExp3Json.h"

/**
 * A support class for importing exp3.json files.
 */
class FCubismExp3JsonImporter
{
public:
	bool ImportFromString(const FString& FileContent);

	void ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismExp3Json>& Json);

private:
	static TSharedPtr<FJsonObject> ParseJSON(const FString& FileContent, bool bSilent);

protected:
	FString Type;

	float FadeInTime = 1.0f;

	float FadeOutTime = 1.0f;

	TArray<FCubismExpressionParameter> Parameters;
};
