/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFrameworkBPLibrary.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

ACubismModel* ULive2DCubismFrameworkBPLibrary::SpawnCubismModel(
	UObject* WorldContextObject,
	UCubismModel3Json* Model3Json,
	const FTransform& Transform,
	const bool bRenderInWorldSpace,
	UTextureRenderTarget2D* RenderTarget
)
{
	if (!WorldContextObject || !Model3Json)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCubismModelFromJson: Invalid context or model asset."));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}

	ACubismModel* ModelActor = World->SpawnActor<ACubismModel>(ACubismModel::StaticClass());
	if (!ModelActor)
	{
		return nullptr;
	}

	ModelActor->Initialize(Model3Json);
	ModelActor->SetActorTransform(Transform);
	ModelActor->Model->bRenderInWorldSpace = bRenderInWorldSpace;
	ModelActor->Model->SetVisibility(bRenderInWorldSpace, true);
	ModelActor->Model->RenderTarget = RenderTarget;

	return ModelActor;
}
