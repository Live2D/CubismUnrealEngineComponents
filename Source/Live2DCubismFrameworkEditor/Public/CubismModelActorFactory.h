/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismMoc3.h"
#include "Model/CubismModelActor.h"
#include "Physics/CubismPhysics3Json.h"
#include "Pose/CubismPose3Json.h"
#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "Expression/CubismExp3Json.h"
#include "Motion/CubismMotion3Json.h"
#include "UserData/CubismUserData3Json.h"
#include "Model/CubismModel3Json.h"
#include "ActorFactories/ActorFactory.h"

#include "CubismModelActorFactory.generated.h"

class UTexture2D;

/**
 * A factory class for spawning ACubismModel actors.
 */
UCLASS()
class UCubismModelActorFactory : public UActorFactory
{
	GENERATED_BODY()

public:
	UCubismModelActorFactory();

protected:
	static void CreateModel(const TObjectPtr<ACubismModel>& ModelActor, const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TObjectPtr<UCubismMoc3> LoadMoc(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TArray<TObjectPtr<UTexture2D>> LoadTextures(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TObjectPtr<UCubismPhysics3Json> LoadPhysics3Json(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TObjectPtr<UCubismPose3Json> LoadPose3Json(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TArray<TObjectPtr<UCubismExp3Json>> LoadExp3Jsons(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TArray<FMotion3JsonGroup> LoadMotion3Jsons(const TObjectPtr<UCubismModel3Json>& Model3Json);
	static TObjectPtr<UCubismUserData3Json> LoadUserData3Json(const TObjectPtr<UCubismModel3Json>& Model3Json);

public:
	//~ Begin UActorFactory Interface
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory Interface
};
