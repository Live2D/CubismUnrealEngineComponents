/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismPartComponent.h"

#include "Model/CubismParameterStoreComponent.h"
#include "Model/CubismModelActor.h"

UCubismPartComponent::UCubismPartComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismPartComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		return;
	}

	if ((Index < 0 || Index >= InModel->GetPartCount()) && !InModel->NonNativePartIds.Contains(Index))
	{
		return;
	}

	if (Model == InModel)
	{
		return;
	}

	Model = InModel;

	if (Index >= 0 && Index < InModel->GetPartCount())
	{
		Id = Model->GetPartId(Index);
		Opacity = Model->GetPartOpacity(Index);
	}
	else
	{
		Id = Model->NonNativePartIds[Index];
		Opacity = 1.0f;
	}

	check(!FGenericPlatformMath::IsNaN(Opacity));
}

void UCubismPartComponent::SetPartOpacity(float TargetOpacity)
{
	Opacity = TargetOpacity;
	if (Model)
	{
		Model->SetPartOpacity(Index, Opacity);
	}
}

// UObject interface
void UCubismPartComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismPartComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismPartComponent, Opacity))
	{
		if (!Model)
		{
			return;
		}
		Model->SetPartOpacity(Index, Opacity);

		if(Model->ParameterStore)
		{
			Model->ParameterStore->SavePartOpacity(Index);
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismPartComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismPartComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}
	Setup(Owner->Model);
}
#endif
// End of UActorComponent interface
