/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismUpdateControllerComponent.h"
#include "CubismUpdatableInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismModel3Json.h"
#include "Model/CubismParameterComponent.h"

UCubismUpdateControllerComponent::UCubismUpdateControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

#if WITH_EDITOR
	bTickInEditor = true;
#endif
}

void UCubismUpdateControllerComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	UE_LOG(LogTemp, Warning, TEXT("UCubismUpdateControllerComponent::OnComponentCreated"));
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCubismUpdateControllerComponent::RefreshUpdatables);
}

void UCubismUpdateControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshUpdatables();
}

void UCubismUpdateControllerComponent::RefreshUpdatables()
{
	Updatables.Empty();

	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(Components);

	for (UActorComponent* Comp : Components)
	{
		if (!Comp)
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("Found Component: %s (%s)"), *Comp->GetName(), *Comp->GetClass()->GetName());

		if (Comp->GetClass()->ImplementsInterface(UCubismUpdatableInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Implements CubismUpdatableInterface: %s"), *Comp->GetName());
		}

		if (Comp->GetClass()->ImplementsInterface(UCubismUpdatableInterface::StaticClass()))
		{
			ICubismUpdatableInterface* InterfacePtr = Cast<ICubismUpdatableInterface>(Comp);
			if (InterfacePtr)
			{
				TScriptInterface<ICubismUpdatableInterface> InterfaceObj;
				InterfaceObj.SetObject(Comp);
				InterfaceObj.SetInterface(InterfacePtr);

				UE_LOG(LogTemp, Warning, TEXT("Registered Updatable: %s"), *Comp->GetName());
				Updatables.Add(InterfaceObj);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ImplementsInterface returned true, but Cast failed: %s"), *Comp->GetName());
			}
		}
	}

	Updatables.Sort([](const TScriptInterface<ICubismUpdatableInterface>& A, const TScriptInterface<ICubismUpdatableInterface>& B)
	{
		return A->GetExecutionOrder() < B->GetExecutionOrder();
	});
}

void UCubismUpdateControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Updatables.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Updatables is empty — trying RefreshUpdatables now"));
		RefreshUpdatables();
	}

	for (const auto& Updatable : Updatables)
	{
		if (Updatable)
		{
			Updatable->OnCubismUpdate(DeltaTime);
		}
	}
}