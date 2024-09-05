/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismRendererComponent.h"

#include "Model/CubismDrawableComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Rendering/CubismMaskTexture.h"
#include "Rendering/CubismMaskTextureComponent.h"
#include "Rendering/CubismMaskJunction.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"

UCubismRendererComponent::UCubismRendererComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismRendererComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	Model = InModel;

	NumMasks = 0;
	Junctions.Empty();

	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
	{
		TSharedPtr<FCubismMaskJunction> TargetJunction = nullptr;

		for (const TSharedPtr<FCubismMaskJunction>& Junction : Junctions)
		{
			if (Junction->MaskDrawables.Num() == Drawable->Masks.Num())
			{
				bool bAny = true;
				for (int32 i = 0, num = Drawable->Masks.Num(); bAny && i < num; i++)
				{
					const int32 MaskDrawableIndex = Drawable->Masks[i];
					bAny &= Junction->MaskDrawables[i] == Model->Drawables[MaskDrawableIndex];
				}

				if (bAny)
				{
					TargetJunction = Junction;
					break;
				}
			}
		}

		if (TargetJunction == nullptr)
		{
			TargetJunction = MakeShared<FCubismMaskJunction>();

			if (Drawable->Masks.Num() > 0)
			{
				TargetJunction->MaskDrawables.Reserve(Drawable->Masks.Num());
				for (const int32 MaskDrawableIndex : Drawable->Masks)
				{
					const TObjectPtr<UCubismDrawableComponent>& MaskDrawable = Model->Drawables[MaskDrawableIndex];
					TargetJunction->MaskDrawables.Add(MaskDrawable);
				}

				NumMasks++;
			}

			Junctions.Add(TargetJunction);
		}

		TargetJunction->Drawables.AddUnique(Drawable);
	}

	Model->Renderer = this;

	ApplyRenderOrder();

	if (MaskTexture)
	{
		MaskTexture->MaskTextureComponent->ResolveMaskLayout();
	}

	AddTickPrerequisiteComponent(Model); // must render after model updated
	AddTickPrerequisiteComponent(MaskTexture->MaskTextureComponent); // must render after mask texture updated
}

void UCubismRendererComponent::ApplyRenderOrder()
{
	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
	{
		int32 NewRenderOrder = Drawable->RenderOrder + Drawable->RenderOrderOffset;

		switch (SortingOrder)
		{
			case ECubismRendererSortingOrder::FrontToBack:
			{
				break;
			}
			case ECubismRendererSortingOrder::BackToFront:
			{
				NewRenderOrder = Model->GetDrawableCount() - NewRenderOrder - 1;

				break;
			}
			default:
			{
				ensure(false);
				break;
			}
		}

		NewRenderOrder += RenderOrder;

		if (bZSort)
		{
			Drawable->SetTranslucentSortPriority(0);
			Drawable->SetRelativeLocation(FVector(NewRenderOrder * Epsilon, 0.0f, 0.0f));
		}
		else
		{
			Drawable->SetTranslucentSortPriority(NewRenderOrder);
			Drawable->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		}
	}
}

// UObject interface
void UCubismRendererComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismRendererComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, MaskTexture))
	{
		if (MaskTexture)
		{
			ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

			MaskTexture->MaskTextureComponent->AddModel(Owner);
		}
	}

	if (
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, SortingOrder) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, bZSort) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, RenderOrder) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, Epsilon))
	{
		ApplyRenderOrder();
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismRendererComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (MaskTexture == nullptr)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), ACubismMaskTexture::StaticClass(), FoundActors);
		if (FoundActors.Num() == 0)
		{
			MaskTexture = Owner->GetWorld()->SpawnActor<ACubismMaskTexture>();
			#if WITH_EDITOR
			MaskTexture->SetActorLabel(TEXT("CubismMaskTexture"));
			MaskTexture->SetFlags(RF_Transactional);
			#endif
		}
		else
		{
			MaskTexture = (ACubismMaskTexture*) FoundActors[0];
		}
	}
	else
	{
		MaskTexture->MaskTextureComponent->RemoveModel(Owner);
	}

	MaskTexture->MaskTextureComponent->AddModel(Owner);

	Setup(Owner->Model);
}

void UCubismRendererComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (MaskTexture)
	{
		ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

		MaskTexture->MaskTextureComponent->RemoveModel(Owner);
	}

	if (Model->Renderer == this)
	{
		Model->Renderer = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UCubismRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const TSharedPtr<FCubismMaskJunction>& Junction : Junctions)
	{
		for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Junction->Drawables)
		{
			UMaterialInstanceDynamic* MaterialInstance = static_cast<UMaterialInstanceDynamic*>(Drawable->GetMaterial(0));

			const TObjectPtr<UTexture2D>& MainTexture   = Drawable->TextureIndex < Model->Textures.Num()? Model->Textures[Drawable->TextureIndex] : nullptr;
			FLinearColor BaseColor     = Drawable->BaseColor;
			FLinearColor MultiplyColor = Drawable->MultiplyColor;
			FLinearColor ScreenColor   = Drawable->ScreenColor;

			{
				if (Model->bOverwriteFlagForModelMultiplyColors)
				{
					MultiplyColor = Model->MultiplyColor;
				}

				if (Model->bOverwriteFlagForModelScreenColors)
				{
					ScreenColor = Model->ScreenColor;
				}
			}

			if (const UCubismPartComponent* ParentPart = Model->GetPart(Drawable->ParentPartIndex))
			{
				if (ParentPart->bOverwriteFlagForPartMultiplyColors)
				{
					MultiplyColor = ParentPart->MultiplyColor;
				}
				
				if (ParentPart->bOverwriteFlagForPartScreenColors)
				{
					ScreenColor = ParentPart->ScreenColor;
				}
			}

			BaseColor.A *= Model->Opacity * Drawable->Opacity;

			MaterialInstance->SetTextureParameterValue("MainTexture", MainTexture);
			MaterialInstance->SetVectorParameterValue("BaseColor", BaseColor);
			MaterialInstance->SetVectorParameterValue("MultiplyColor", MultiplyColor);
			MaterialInstance->SetVectorParameterValue("ScreenColor", ScreenColor);

			if (Drawable->IsMasked())
			{
				MaterialInstance->SetTextureParameterValue("MaskTexture", Junction->RenderTarget);
				MaterialInstance->SetVectorParameterValue("Offset", Junction->Offset);
				MaterialInstance->SetVectorParameterValue("Channel", Junction->Channel);
			}
		}
	}
}
// End of UActorComponent interface
