/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "UserData/CubismUserDataComponent.h"

#include "UserData/CubismUserData3Json.h"
#include "Model/CubismDrawableComponent.h"
#include "Model/CubismModelActor.h"

void UCubismUserDataComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	Model = InModel;

	if (!Json)
	{
		for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
		{
			Drawable->UserDataTag = TEXT("");
		}
		return;
	}
	
	const FCubismUserDataEntry& UserDataEntry = Json->Data[ECubismUserDataTargetType::ArtMesh];

	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
	{
		if (UserDataEntry.Tags.Contains(Drawable->Id))
		{
			Drawable->UserDataTag = UserDataEntry.Tags[Drawable->Id];
		}
		else
		{
			Drawable->UserDataTag = TEXT("");
		}
	}
}

// UObject interface
void UCubismUserDataComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismUserDataComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismUserDataComponent, Json))
	{
		Setup(Model);
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismUserDataComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
// End of UActorComponent interface
