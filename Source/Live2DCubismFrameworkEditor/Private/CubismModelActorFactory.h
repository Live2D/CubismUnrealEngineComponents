/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

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

public:
	//~ Begin UActorFactory Interface
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory Interface
};
