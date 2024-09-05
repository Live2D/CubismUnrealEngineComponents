/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Pose/CubismPoseComponent.h"

#include "Model/CubismModelComponent.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Pose/CubismPose3Json.h"

const float Epsilon = 0.001f;
const float DefaultFadeInSeconds = 0.5f;

const float Phi = 0.5f;
const float BackOpacityThreshold = 0.15f;

UCubismPoseComponent::UCubismPoseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismPoseComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	FadeInTime = DefaultFadeInSeconds;

	if (Json)
	{
		PartGroups.Empty();

		FadeInTime = Json->FadeInTime;

		if (FadeInTime < 0.0f)
		{
			FadeInTime = DefaultFadeInSeconds;
		}

		for (const FCubismPosePartGroup& PartGroup : Json->PartGroups)
		{
			FCubismPosePartGroupParameter Group;

			for (const FCubismPosePart& Part : PartGroup.Parts)
			{
				FCubismPosePartParameter PartParam;

				PartParam.Part = Model->GetPart(Part.Id);
				PartParam.Parameter = Model->GetParameter(Part.Id);

				for (const FString& LinkId : Part.Links)
				{
					if (const TObjectPtr<UCubismPartComponent>& LinkPart = Model->GetPart(LinkId))
					{
						PartParam.LinkParts.Add(LinkPart);
					}
				}

				Group.Parts.Add(PartParam);
			}

			PartGroups.Add(Group);
		}
	}

	Model->Pose = this;

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	AddTickPrerequisiteComponent(Model->Motion); // must be updated at first because motions overwrite parameters
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

// UObject interface
void UCubismPoseComponent::PostLoad()
{
	Super::PostLoad();

	ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismPoseComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismPoseComponent, Json))
	{
		Setup(Model);
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismPoseComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismPoseComponent::DoFade(float DeltaTime)
{
	for (const FCubismPosePartGroupParameter& PartGroup : PartGroups)
	{
		check(PartGroup.Parts.Num() > 0);

		// default visible part and its opacity
		TObjectPtr<UCubismPartComponent> VisiblePart = PartGroup.Parts[0].Part;

		check(VisiblePart);

		float NewOpacity = 1.0f;

		// find the visible part in a group
		for (const FCubismPosePartParameter& PartParam : PartGroup.Parts)
		{
			const TObjectPtr<UCubismPartComponent>& Part = PartParam.Part;
			const TObjectPtr<UCubismParameterComponent>& Parameter = PartParam.Parameter;

			if (!Part || !Parameter || !(Parameter->Value > Epsilon))
			{
				continue;
			}

			if (Parameter->Value > 0.0f)
			{
				VisiblePart = Part;
				NewOpacity = Part->Opacity;

				break;
			}
		}

		// FIXME: There is a possibility of division by zero, but the fix will be applied after addressing the issue in other SDKs.
		NewOpacity += DeltaTime / FadeInTime;

		if (NewOpacity > 1.0f)
		{
			NewOpacity = 1.0f;
		}

		for (const FCubismPosePartParameter& PartParam : PartGroup.Parts)
		{
			const TObjectPtr<UCubismPartComponent>& Part = PartParam.Part;

			if (!Part)
			{
				continue;
			}

			//  The settings for the visible parts.
			if (Part == VisiblePart)
			{
				Part->SetPartOpacity(NewOpacity);
			}
			// The settings for the hidden parts.
			else
			{
				float Opacity = Part->Opacity;
				float A1;          // The opacity calculated through the computation.

				if (NewOpacity < Phi)
				{
					A1 = NewOpacity * (Phi - 1.0f) / Phi + 1.0f; // a line through (0, 1) and (phi, phi)
				}
				else
				{
					A1 = (1.0f - NewOpacity) * Phi / (1.0f - Phi); // a line through (1, 0) and (phi, phi)
				}

				const float BackOpacity = (1.0f - A1) * (1.0f - NewOpacity);

				if (BackOpacity > BackOpacityThreshold)
				{
					A1 = 1.0f - BackOpacityThreshold / (1.0f - NewOpacity);
				}

				if (Opacity > A1)
				{
					Opacity = A1; // If the calculated opacity is greater (more opaque) than the computed opacity, increase the opacity.
				}

				Part->SetPartOpacity(Opacity);
			}
		}
	}
}

void UCubismPoseComponent::CopyPartOpacities()
{
	// apply opacity to linked parts
	for (const FCubismPosePartGroupParameter& PartGroup : PartGroups)
	{
		for (const FCubismPosePartParameter& PartParam : PartGroup.Parts)
		{
			if (PartParam.LinkParts.Num() == 0)
			{
				continue; // no linked parts
			}

			const float Opacity = PartParam.Part->Opacity;

			for (const TObjectPtr<UCubismPartComponent>& LinkPart : PartParam.LinkParts)
			{
				LinkPart->SetPartOpacity(Opacity);
			}
		}
	}
}

void UCubismPoseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DoFade(DeltaTime);

	CopyPartOpacities();
}
// End of UActorComponent interface
