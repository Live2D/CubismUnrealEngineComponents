/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismRendererComponent.h"

#include "CubismMaskRenderer.h"
#include "CubismUpdateExecutionOrder.h"
#include "Model/CubismDrawableComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Rendering/CubismMaskTexture.h"
#include "Rendering/CubismMaskTextureComponent.h"
#include "Rendering/CubismMaskJunction.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "SceneInterface.h"

UCubismRendererComponent::UCubismRendererComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismRendererComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismRendererComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

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
					bAny &= Junction->MaskDrawables[i].Drawable == Model->Drawables[MaskDrawableIndex];
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
					const TObjectPtr<UCubismDrawableComponent>& DrawableData = Model->Drawables[MaskDrawableIndex];
					TUniquePtr<FCubismMaskRenderer> MaskRenderer = MakeUnique<FCubismMaskRenderer>(DrawableData->GetVertexPositions().Num(), DrawableData->GetVertexIndices().Num());

					FCubismMaskJunction::FMaskDrawableData MaskDrawable{ .Drawable = DrawableData, .Renderer = MoveTemp(MaskRenderer) };

					TargetJunction->MaskDrawables.Add(MoveTemp(MaskDrawable));
				}

				NumMasks++;
			}

			Junctions.Add(TargetJunction);
		}

		TargetJunction->Drawables.AddUnique(Drawable);
	}

	if (Model->Renderer != this)
	{
		if (Model->Renderer)
		{
			Model->Renderer->DestroyComponent();
		}
		Model->Renderer = this;
	}

	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
	{
		const int32 NewRenderOrder = CalcRenderOrder(Drawable);

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

	if (MaskTexture)
	{
		ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

		if (Owner && MaskTexture->MaskTextureComponent)
		{
			MaskTexture->MaskTextureComponent->AddModel(Owner);
			MaskTexture->MaskTextureComponent->ResolveMaskLayout();
			AddTickPrerequisiteComponent(MaskTexture->MaskTextureComponent); // must render after mask texture updated
		}
	}

	AddTickPrerequisiteComponent(Model); // must render after model updated
}

int32 UCubismRendererComponent::CalcRenderOrder(const UCubismDrawableComponent* Drawable) const
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

	return NewRenderOrder;
}

// UObject interface
void UCubismRendererComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());
	if (!Owner || !Owner->Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Owner or Model."));
		return;
	}

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismRendererComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!Model)
	{
		return;
	}

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, MaskTexture))
	{
		if (MaskTexture)
		{
			ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

			if (Owner && MaskTexture->MaskTextureComponent)
			{
				MaskTexture->MaskTextureComponent->AddModel(Owner);
			}
		}
	}

	if (
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, SortingOrder) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, bZSort) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, RenderOrder) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCubismRendererComponent, Epsilon))
	{
		for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Model->Drawables)
		{
			const int32 NewRenderOrder = CalcRenderOrder(Drawable);

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
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismRendererComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	if (!Owner)
	{
		return;
	}

	if (!Owner->Model)
	{
		return;
	}

	if (MaskTexture == nullptr)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), ACubismMaskTexture::StaticClass(), FoundActors);
		if (FoundActors.Num() == 0)
		{
			MaskTexture = Owner->GetWorld()->SpawnActor<ACubismMaskTexture>();
			if (MaskTexture)
			{
#if WITH_EDITOR
				MaskTexture->SetActorLabel(TEXT("CubismMaskTexture"));
				MaskTexture->SetFlags(RF_Transactional);
#endif
			}
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

	Setup(Owner->Model);
}

void UCubismRendererComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (!Model) return;

	if (MaskTexture)
	{
		ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

		MaskTexture->MaskTextureComponent->RemoveModel(Owner);
	}

	if (Model && Model->Renderer == this)
	{
		Model->Renderer = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismRendererComponent::PostEditUndo()
{
	Super::PostEditUndo();

	ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

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

void UCubismRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	OnCubismUpdate(DeltaTime);
}

void UCubismRendererComponent::OnCubismUpdate(float DeltaTime)
{
	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("Model is null."));
		return;
	}

	for (const TSharedPtr<FCubismMaskJunction>& Junction : Junctions)
	{
		for (const TObjectPtr<UCubismDrawableComponent>& MaskJunctionDrawable : Junction->Drawables)
		{
			UMaterialInstanceDynamic* MaterialInstance = static_cast<UMaterialInstanceDynamic*>(MaskJunctionDrawable->GetMaterial(0));

			const TObjectPtr<UTexture2D>& MainTexture   = MaskJunctionDrawable->TextureIndex < Model->Textures.Num()? Model->Textures[MaskJunctionDrawable->TextureIndex] : nullptr;
			FLinearColor BaseColor     = MaskJunctionDrawable->BaseColor;
			FLinearColor MultiplyColor = MaskJunctionDrawable->MultiplyColor;
			FLinearColor ScreenColor   = MaskJunctionDrawable->ScreenColor;

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

			if (const UCubismPartComponent* ParentPart = Model->GetPart(MaskJunctionDrawable->ParentPartIndex))
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

			BaseColor.A *= Model->Opacity * MaskJunctionDrawable->Opacity;

			MaterialInstance->SetTextureParameterValue("MainTexture", MainTexture);
			MaterialInstance->SetVectorParameterValue("BaseColor", BaseColor);
			MaterialInstance->SetVectorParameterValue("MultiplyColor", MultiplyColor);
			MaterialInstance->SetVectorParameterValue("ScreenColor", ScreenColor);

			if (MaskJunctionDrawable->IsMasked())
			{
				MaterialInstance->SetTextureParameterValue("MaskTexture", Junction->RenderTarget);
				MaterialInstance->SetVectorParameterValue("Offset", Junction->Offset);
				MaterialInstance->SetVectorParameterValue("Channel", Junction->Channel);
			}
		}
	}
}

int32 UCubismRendererComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_RENDERER;
}
// End of UActorComponent interface
