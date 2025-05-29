/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Rendering/CubismMaskTextureComponent.h"

#include "Model/CubismDrawableComponent.h"
#include "Rendering/CubismRendererComponent.h"
#include "Rendering/CubismMaskJunction.h"
#include "Rendering/CubismMaskShaders.h"
#include "Engine/Texture2D.h"
#include "CubismLog.h"
#include <math.h>

UCubismMaskTextureComponent::UCubismMaskTextureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismMaskTextureComponent::AddModel(ACubismModel* ModelActor)
{
	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : ModelActor->Model->Drawables)
	{
		AddTickPrerequisiteComponent(Drawable); // must be updated after drawable updated
	}

	Models.AddUnique(ModelActor);

	for (int32 i = Models.Num() - 1; i >= 0; --i)
	{
		if (!IsValid(Models[i]))
		{
			Models.RemoveAt(i);
		}
	}

	bDirty = true;
}

void UCubismMaskTextureComponent::RemoveModel(ACubismModel* ModelActor)
{
	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : ModelActor->Model->Drawables)
	{
		RemoveTickPrerequisiteComponent(Drawable);
	}

	Models.Remove(ModelActor);

	for (int32 i = Models.Num() - 1; i >= 0; --i)
	{
		if (!IsValid(Models[i]))
		{
			Models.RemoveAt(i);
		}
	}

	bDirty = true;
}

void UCubismMaskTextureComponent::ResolveMaskLayout()
{
	NumMasks = 0;
	for (const TObjectPtr<ACubismModel>& ModelActor : Models)
	{
		if (IsValid(ModelActor) && IsValid(ModelActor->Model) && IsValid(ModelActor->Model->Renderer))
		{
			NumMasks += ModelActor->Model->Renderer->NumMasks;
			ModelActor->Model->Renderer->MaskTexture = (ACubismMaskTexture*) GetOwner();
		}
	}

	if (!bUseMultiRenderTargets)
	{
		RenderTargetCount = 1;
		LOD = floor(0.5f*log2(fmaxf(1, NumMasks-1)));
	}

	const int32 Resolution = 1<<LOD, LayoutSize = 1<<(LOD<<1);

	AllocateRenderTargets(RenderTargetCount);

	int32 Index = 0;
	for (const TObjectPtr<ACubismModel>& ModelActor : Models)
	{
		if (!IsValid(ModelActor) || !IsValid(ModelActor->Model) || !IsValid(ModelActor->Model->Renderer))
		{
			continue;
		}

		for (const TSharedPtr<FCubismMaskJunction>& Junction : ModelActor->Model->Renderer->Junctions)
		{
			if (!Junction || Junction->MaskDrawables.Num() == 0)
			{
				continue;
			}

			// Index = 4 * (LayoutSize * RenderTargetIndex + LayoutIndex) + Channel
			const int32 Channel = Index % 4;
			const int32 LayoutIndex = (Index / 4) % LayoutSize;
			const int32 RenderTargetIndex = (Index / 4) / LayoutSize;

			const int32 Column = LayoutIndex % Resolution;
			const int32 Row = LayoutIndex / Resolution;

			if (RenderTargets.IsValidIndex(RenderTargetIndex))
			{
				Junction->RenderTarget =  RenderTargets[RenderTargetIndex];
			}
			else
			{
				Junction->RenderTarget = nullptr;

				UE_LOG(LogCubism, Error, TEXT("The mask(%d) is not be drawn correctly because the number of render targets is not enough."), Index);
			}

			/*
			 * The formula to arrange the vertex position (x, y) \in [-1,1]^2 along the mask layout position (c, r) is:
			 * x' = x/R + (2c+1)/R - 1
			 * y' = y/R + (2r+1)/R - 1
			 * The formula to convert the scale [-1,1]^2 to the UV space [0,1]^2 is:
			 * u = (x'+1)/2
			 * v = (y'+1)/2
			 * Combining these two formulas, we get:
			 * u = (x+2c+1)/2R
			 * v = (y+2r+1)/2R
			 */
			if (ModelActor->Model)
			{
				Junction->Offset = FVector4(
					2.0f * Column + 1.0f,
					2.0f * Row + 1.0f,
					0.5f / Resolution,
					100.0f / ModelActor->Model->GetPixelsPerUnit()
				);
			}

			if (Channel%4 == 0)
			{
				Junction->Channel = FVector4(1, 0, 0, 0);
			}
			else if (Channel%4 == 1)
			{
				Junction->Channel = FVector4(0, 1, 0, 0);
			}
			else if (Channel%4 == 2)
			{
				Junction->Channel = FVector4(0, 0, 1, 0);
			}
			else
			// if (Channel%4 == 3)
			{
				Junction->Channel = FVector4(0, 0, 0, 1);
			}

			Index++;
		}
	}
}

void UCubismMaskTextureComponent::AllocateRenderTargets(const int32 RequiredRTs)
{
	const int32 Diff = RequiredRTs - RenderTargets.Num();
	if (Diff > 0)
	{
		for (int32 i = 0; i < Diff; i++)
		{
			UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(
				this,
				*FString::Printf(TEXT("MaskRenderTarget_%d"), RenderTargets.Num()),
				RF_Transactional
			);
			check(RenderTarget);
			RenderTarget->RenderTargetFormat = RTF_RGBA8;
			RenderTarget->ClearColor = FLinearColor::Transparent;
			RenderTarget->bAutoGenerateMips = false;
			RenderTarget->InitAutoFormat(Size, Size);
			RenderTarget->UpdateResourceImmediate(true);

			RenderTargets.Add(RenderTarget);
		}
	}
	else
	{
		for (int32 i = 0; i < -Diff; i++)
		{
			UTextureRenderTarget2D* OldRT = RenderTargets.Pop();
			if (OldRT)
			{
				OldRT->ConditionalBeginDestroy();
				OldRT = nullptr;
			}
		}
	}
}

// UObject interface
#if WITH_EDITOR
void UCubismMaskTextureComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMaskTextureComponent, bUseMultiRenderTargets))
	{
		bDirty = true;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMaskTextureComponent, Size))
	{
		bDirty = true;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMaskTextureComponent, RenderTargetCount))
	{
		bDirty = true;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMaskTextureComponent, LOD))
	{
		bDirty = true;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismMaskTextureComponent, Models))
	{
		bDirty = true;
	}

	if (bDirty)
	{
		MarkPackageDirty();
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismMaskTextureComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	ResolveMaskLayout();
}

void UCubismMaskTextureComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	RenderTargets.Empty();
}

#if WITH_EDITOR
void UCubismMaskTextureComponent::PostEditUndo()
{
	Super::PostEditUndo();

	ResolveMaskLayout();
}
#endif

void UCubismMaskTextureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDirty)
	{
		ResolveMaskLayout();
		bDirty = false;
	}

	for (const TObjectPtr<UTextureRenderTarget2D>& RenderTarget : RenderTargets)
	{
		if (!IsValid(RenderTarget))
		{
			continue;
		}

		TArray<FMaskDrawInfo> MaskDrawInfos;

		for (const TObjectPtr<ACubismModel>& ModelActor : Models)
		{
			if (!IsValid(ModelActor) || !IsValid(ModelActor->Model) || !IsValid(ModelActor->Model->Renderer))
			{
				continue;
			}

			const TArray<TObjectPtr<UTexture2D>>& Textures = ModelActor->Model->Textures;

			for (const TSharedPtr<FCubismMaskJunction>& Junction : ModelActor->Model->Renderer->Junctions)
			{
				// If the render target does not match, skip drawing the mask.
				if (Junction->RenderTarget != RenderTarget)
				{
					continue;
				}

				for (const TObjectPtr<UCubismDrawableComponent>& MaskDrawable : Junction->MaskDrawables)
				{
					// If the texture does not exist, skip drawing the mask. 
					if (MaskDrawable->TextureIndex >= Textures.Num())
					{
						continue;
					}

					const TObjectPtr<UTexture2D>& Texture = Textures[MaskDrawable->TextureIndex];

					if (!Texture)
					{
						continue;
					}

					const TArray<int32>& Indices = MaskDrawable->GetVertexIndices();
					const TArray<FVector2D>& Positions = MaskDrawable->GetVertexPositions();
					const TArray<FVector2D>& UVs = MaskDrawable->GetVertexUvs();

					FMaskDrawInfo DrawInfo;

					for (int32 i = 0; i < Indices.Num(); ++i)
					{
						DrawInfo.Indices.Add((uint16)Indices[i]);
					}
					for (int32 i = 0; i < Positions.Num(); ++i)
					{
						FCubismMeshMaskVertex Vertex;

						Vertex.Position = (FVector2f)Positions[i];
						Vertex.UV = (FVector2f)UVs[i];

						DrawInfo.Vertices.Add(Vertex);
					}
					DrawInfo.Offset = Junction->Offset;
					DrawInfo.Channel = Junction->Channel;
					DrawInfo.MainTexture = Texture->GetResource();

					MaskDrawInfos.Add(DrawInfo);
				}
			}
		}

		FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

		// Draw the mask to the render target.
		ENQUEUE_RENDER_COMMAND(DrawMaskCommand)(
			[this, RenderTargetResource, MaskDrawInfos](FRHICommandList& RHICmdList)
			{
				DrawCubismMeshMask_RenderThread(RHICmdList, RenderTargetResource, MaskDrawInfos);
			}
		);
	}
}
// End of UActorComponent interface
