/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismDrawableComponent.h"

#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Rendering/CubismDrawableSceneProxy.h"
#include "UserData/CubismUserData3Json.h"
#include "CubismLog.h"
#include "Live2DCubismCore.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/Vector.h"

UCubismDrawableComponent::UCubismDrawableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
	bBoundsDirty = true;
}

void UCubismDrawableComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		return;
	}

	check(Index >= 0 && Index < InModel->GetDrawableCount());

	if (Model == InModel)
	{
		return;
	}

	Model = InModel;

	Id = Model->GetDrawableId(Index);

	RenderOrder = Model->GetDrawableRenderOrder(Index);

	TextureIndex = Model->GetDrawableTextureIndex(Index);

	{
		const int32 VertexIndexCount(Model->GetDrawableVertexIndexCount(Index));
		const int32 VertexCount(Model->GetDrawableVertexCount(Index));
		const int32 MaskCount(Model->GetDrawableMaskCount(Index));

		const csmVector2* DrawableVertexUvs(Model->GetDrawableVertexUv(Index));
		VertexUvs.Empty();
		VertexUvs.Reserve(VertexCount);

		for (int32 i = 0; i < VertexCount; i++)
		{
			VertexUvs.Add(FVector2D(DrawableVertexUvs[i].X, DrawableVertexUvs[i].Y));
		}

		const csmVector2* DrawableVertexPositions(Model->GetDrawableVertexPosition(Index));
		VertexPositions.Empty();
		VertexPositions.Reserve(VertexCount);

		for (int32 i = 0; i < VertexCount; i++)
		{
			VertexPositions.Add(FVector2D(-DrawableVertexPositions[i].X, DrawableVertexPositions[i].Y));
		}

		const uint16* DrawableVertexIndices(Model->GetDrawableVertexIndex(Index));
		VertexIndices.Empty();
		VertexIndices.Reserve(VertexIndexCount);

		for (int32 i = 0; i < VertexIndexCount; ++i)
		{
			VertexIndices.Add(static_cast<int32>(DrawableVertexIndices[i]));
		}

		const int32* DrawableMasks(Model->GetDrawableMask(Index));
		Masks.Empty();
		Masks.Reserve(MaskCount);
		for (int32 MaskIndex = 0; MaskIndex < MaskCount; ++MaskIndex)
		{
			Masks.Add(DrawableMasks[MaskIndex]);
		}
	}

	Opacity = Model->GetDrawableOpacity(Index);
	BaseColor = FLinearColor::White;
	MultiplyColor = Model->GetDrawableMultiplyColor(Index);
	ScreenColor = Model->GetDrawableScreenColor(Index);
	bTwoSided = Model->GetDrawableIsTwoSided(Index);
	UserMultiplyColor = MultiplyColor;
	UserScreenColor = ScreenColor;
	bUserTwoSided = bTwoSided;

	bOverwriteFlagForDrawableMultiplyColors = false;
	bOverwriteFlagForDrawableScreenColors = false;
	bOverwriteFlagForDrawableIsTwoSided = false;

	ParentPartIndex = Model->GetDrawableParentPartIndex(Index);
	BlendMode = Model->GetDrawableBlendMode(Index);
	InvertedMask = Model->GetDrawableInvertedMask(Index);

	FString MaterialName;

	switch(BlendMode)
	{
		case ECubismDrawableBlendMode::Normal:
		{
			MaterialName = !IsMasked()? TEXT("CustomUnlitNormal") : InvertedMask? TEXT("CustomUnlitNormalMaskedInverted") : TEXT("CustomUnlitNormalMasked");
			break;
		}
		case ECubismDrawableBlendMode::Additive:
		{
			MaterialName = !IsMasked()? TEXT("CustomUnlitAdditive") : InvertedMask? TEXT("CustomUnlitAdditiveMaskedInverted") : TEXT("CustomUnlitAdditiveMasked");
			break;
		}
		case ECubismDrawableBlendMode::Multiplicative:
		{
			MaterialName = !IsMasked()? TEXT("CustomUnlitMultiplicative") : InvertedMask? TEXT("CustomUnlitMultiplicativeMaskedInverted") : TEXT("CustomUnlitMultiplicativeMasked");
			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}

	UMaterial* Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *(TEXT("/Live2DCubismSDK/Materials") / MaterialName)));
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(Material, this, *MaterialName);

	SetMaterial(0, static_cast<UMaterialInterface*>(MaterialInstance));

	if (!Model->UserDataJson)
	{
		UserDataTag = TEXT("");
	}
	else
	{
		const FCubismUserDataEntry& UserDataEntry = Model->UserDataJson->Data[ECubismUserDataTargetType::ArtMesh];

		if (UserDataEntry.Tags.Contains(Id))
		{
			UserDataTag = UserDataEntry.Tags[Id];
		}
		else
		{
			UserDataTag = TEXT("");
		}
	}

	AddTickPrerequisiteComponent(Model); // must be updated after model updated
}

TArray<int32> UCubismDrawableComponent::GetVertexIndices() const
{
	return VertexIndices;
}

TArray<FVector2D> UCubismDrawableComponent::GetVertexPositions() const
{
	return VertexPositions;
}

FVector UCubismDrawableComponent::ToGlobalPosition(const FVector2D VertexPosition) const
{
	const float Scale = 0.01f * Model->GetPixelsPerUnit();

	// align the model on the y-z plane
	return FVector(0.0f, Scale * VertexPosition.X, Scale * VertexPosition.Y);
}

TArray<FVector2D> UCubismDrawableComponent::GetVertexUvs() const
{
	return VertexUvs;
}


const TArray<int32> UCubismDrawableComponent::GetDrawableMask() const
{
	return TArray<int32>(Model->GetDrawableMask(Index), Model->GetDrawableMaskCount(Index));
}

int32 UCubismDrawableComponent::GetDrawableMaskCount() const
{
	return Model->GetDrawableMaskCount(Index);
}

// UObject interface
void UCubismDrawableComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismDrawableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, bOverwriteFlagForDrawableMultiplyColors))
	{
		// If the flag is changed from false to true, the stored color is applied.
		if (bOverwriteFlagForDrawableMultiplyColors)
		{
			MultiplyColor = UserMultiplyColor;
		}
		// If the flag is changed from true to false, the color from the model is applied.
		else
		{
			MultiplyColor = Model->GetDrawableMultiplyColor(Index);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, MultiplyColor))
	{
		UserMultiplyColor = MultiplyColor;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, bOverwriteFlagForDrawableScreenColors))
	{
		// If the flag is changed from false to true, the stored color is applied.
		if (bOverwriteFlagForDrawableScreenColors)
		{
			ScreenColor = UserScreenColor;
		}
		// If the flag is changed from true to false, the color from the model is applied.
		else
		{
			ScreenColor = Model->GetDrawableScreenColor(Index);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, ScreenColor))
	{
		UserScreenColor = ScreenColor;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, bOverwriteFlagForDrawableIsTwoSided))
	{
		// If the flag is changed from false to true, the stored value is applied.
		if (bOverwriteFlagForDrawableIsTwoSided)
		{
			bTwoSided = bUserTwoSided;
		}
		// If the flag is changed from true to false, the value from the model is applied.
		else
		{
			bTwoSided = Model->GetDrawableIsTwoSided(Index);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismDrawableComponent, bTwoSided))
	{
		bUserTwoSided = bTwoSided;
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismDrawableComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismDrawableComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismDrawableComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	if (FCubismDrawableSceneProxy* DrawableProxy = static_cast<FCubismDrawableSceneProxy*>(SceneProxy))
	{
		FCubismDrawableDynamicMeshData NewDynamicData;

		NewDynamicData.Index = Index;

		for (const FVector2D& LocalPosition : GetVertexPositions())
		{
			NewDynamicData.Positions.Add(FVector3f(ToGlobalPosition(LocalPosition)));
		}

		for (const FVector2D& UV : GetVertexUvs())
		{
			NewDynamicData.UVs.Add(FVector2f(UV));
		}

		NewDynamicData.Indices.Append(VertexIndices);
		NewDynamicData.bTwoSided = bTwoSided;

		ENQUEUE_RENDER_COMMAND(DrawableUpdateDynamicData)(
			[DrawableProxy, NewDynamicData](FRHICommandListImmediate& RHICmdList)
			{
				DrawableProxy->UpdateDynamicData(RHICmdList, NewDynamicData);
			}
		);
	}
}

void UCubismDrawableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Model)
	{
		return;
	}

	if (Model->GetDrawableDynamicFlagOpacityDidChange(Index))
	{
		Opacity = Model->GetDrawableOpacity(Index);
	}

	if (Model->GetDrawableDynamicFlagVertexPositionsDidChange(Index))
	{
		VertexPositions.Empty();

		const csmVector2* DrawableVertexPositions(Model->GetDrawableVertexPosition(Index));
		const int32 VertexCount(Model->GetDrawableVertexCount(Index));
		VertexPositions.Reserve(VertexCount);

		for (int32 i = 0; i < VertexCount; i++)
		{	
			VertexPositions.Add(FVector2D(-DrawableVertexPositions[i].X, DrawableVertexPositions[i].Y));
		}

		bBoundsDirty = true;
		MarkRenderDynamicDataDirty();
	}

	if (Model->GetDrawableDynamicFlagBlendColorDidChange(Index))
	{
		if (!bOverwriteFlagForDrawableMultiplyColors)
		{
			MultiplyColor = Model->GetDrawableMultiplyColor(Index);
		}

		if (!bOverwriteFlagForDrawableScreenColors)
		{
			ScreenColor = Model->GetDrawableScreenColor(Index);
		}
	}
}
// End of UActorComponent interface

//~ Begin USceneComponent Interface
FBoxSphereBounds UCubismDrawableComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (Model)
	{
		if (bBoundsDirty)
		{
			FBox Box(ForceInit);

			for (const FVector2D& LocalPosition : GetVertexPositions())
			{
				Box += ToGlobalPosition(LocalPosition);
			}

			LocalBounds = FBoxSphereBounds(Box);
			bBoundsDirty = false;
		}
	}

	return LocalBounds.TransformBy(LocalToWorld);
}
//~ End USceneComponent Interface

//~ Begin UPrimitiveComponent Interface
FPrimitiveSceneProxy* UCubismDrawableComponent::CreateSceneProxy()
{
	FCubismDrawableDynamicMeshData DynamicData;

	DynamicData.Index = Index;

	for (const FVector2D& LocalPosition : GetVertexPositions())
	{
		DynamicData.Positions.Add(FVector3f(ToGlobalPosition(LocalPosition)));
	}

	for (const FVector2D& UV : GetVertexUvs())
	{
		DynamicData.UVs.Add(FVector2f(UV));
	}

	DynamicData.Indices.Append(VertexIndices);
	DynamicData.bTwoSided = bTwoSided;

	return new FCubismDrawableSceneProxy(this, DynamicData);
}
//~ End UPrimitiveComponent Interface
