/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Physics/CubismPhysics3Json.h"

/**
 * A support class for importing physics3.json files.
 */
class FCubismPhysics3JsonImporter
{
public:
	bool ImportFromString(const FString& FileContent);

	void ApplyParams(EObjectFlags Flags, const TObjectPtr<UCubismPhysics3Json>& Json);

private:
	static TSharedPtr<FJsonObject> ParseJSON(const FString& FileContent, bool bSilent);

protected:
	int32 Version;

	FVector2D Gravity = FVector2D(0.0f, -1.0f);

	FVector2D Wind = FVector2D(0.0f, 0.0f);

	float Fps = 0.0f;

	int32 TotalInputCount = 0;

	int32 TotalOutputCount = 0;

	int32 TotalParticleCount = 0;

	TArray<FCubismPhysicsSetting> PhysicsSettings;

private:
	TMap<FString, FText> PhysicsSettingNameMap;

};
