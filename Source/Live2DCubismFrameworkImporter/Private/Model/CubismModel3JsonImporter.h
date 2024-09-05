/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CoreMinimal.h"
#include "Model/CubismModel3Json.h"

/**
 * A support class for importing model3.json files.
 */
class FCubismModel3JsonImporter
{
public:
	bool ImportFromString(const FString& FileContent);

	bool ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismModel3Json>& ModelSetting);

private:
	static TSharedPtr<FJsonObject> ParseJSON(const FString& FileContent, bool bSilent);

protected:
	int32 Version;

	FString MocPath;

	TArray<FString> TexturePaths;

	FString PhysicsPath;

	FString PosePath;

	FString DisplayInfoPath;

	TArray<FExpressionEntry> Expressions;

	TArray<FMotionGroupEntry> Motions;

	FString UserDataPath;

	TArray<FString> EyeBlinks;

	TArray<FString> LipSyncs;

	TArray<FHitAreaEntry> HitAreas;
};
