/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Model/CubismModelComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Model/CubismMoc3.h"
#include "Model/CubismDrawableComponent.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismParameterStoreComponent.h"
#include "Rendering/CubismRendererComponent.h"
#include "Motion/CubismMotionComponent.h"
#include "Expression/CubismExpressionComponent.h"
#include "Physics/CubismPhysicsComponent.h"
#include "Pose/CubismPoseComponent.h"
#include "Effects/EyeBlink/CubismEyeBlinkComponent.h"
#include "Effects/HarmonicMotion/CubismHarmonicMotionComponent.h"
#include "Effects/LipSync/CubismLipSyncComponent.h"
#include "Effects/LookAt/CubismLookAtComponent.h"
#include "Effects/Raycast/CubismRaycastComponent.h"
#include "UserData/CubismUserData3Json.h"

#include "CubismLog.h"
#include <CubismUpdateControllerComponent.h>

UCubismModelComponent::UCubismModelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

UCubismModelComponent::~UCubismModelComponent()
{
}

void UCubismModelComponent::Setup()
{
	if (!Moc)
	{
		return;
	}

	Moc->SetupModel(this);

	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Drawables)
	{
		Drawable->Setup(this);
	}

	for (const TObjectPtr<UCubismParameterComponent>& Parameter : Parameters)
	{
		Parameter->Setup(this);
	}

	for (const TObjectPtr<UCubismPartComponent>& Part : Parts)
	{
		Part->Setup(this);
	}

	SetVisibility(bRenderInWorldSpace, true);
}

////

FVector2D UCubismModelComponent::GetCanvasSize() const
{
	float PixelsPerUnit;
	csmVector2 CanvasSize, CanvasOrigin;

	csmReadCanvasInfo(RawModel, &CanvasSize, &CanvasOrigin, &PixelsPerUnit);

	return FVector2D(CanvasSize.X, CanvasSize.Y);
}

FVector2D UCubismModelComponent::GetCanvasOrigin() const
{
	float PixelsPerUnit;
	csmVector2 CanvasSize, CanvasOrigin;

	csmReadCanvasInfo(RawModel, &CanvasSize, &CanvasOrigin, &PixelsPerUnit);

	return FVector2D(CanvasOrigin.X, CanvasOrigin.Y);
}

float UCubismModelComponent::GetPixelsPerUnit() const
{
	float PixelsPerUnit;
	csmVector2 CanvasSize, CanvasOrigin;

	csmReadCanvasInfo(RawModel, &CanvasSize, &CanvasOrigin, &PixelsPerUnit);

	return PixelsPerUnit;
}

////

int32 UCubismModelComponent::GetDrawableCount() const
{
	return csmGetDrawableCount(RawModel);
}

FString UCubismModelComponent::GetDrawableId(const int32 DrawableIndex) const
{
	check(0 <= DrawableIndex && DrawableIndex < GetDrawableCount());

	const char* DrawableId = csmGetDrawableIds(RawModel)[DrawableIndex];

	return FString(UTF8_TO_TCHAR(DrawableId));
}

int32 UCubismModelComponent::GetDrawableIndex(const FString DrawableId) const
{
	if (!DrawableIndices.Contains(DrawableId))
	{
		return -1;
	}

	return DrawableIndices[DrawableId];
}

UCubismDrawableComponent* UCubismModelComponent::GetDrawable(const int32 DrawableIndex) const
{
	return Drawables.IsValidIndex(DrawableIndex)? Drawables[DrawableIndex] : nullptr;
}

UCubismDrawableComponent* UCubismModelComponent::GetDrawable(const FString DrawableId) const
{
	return !DrawableId.IsEmpty() && DrawableIndices.Contains(DrawableId)? Drawables[DrawableIndices[DrawableId]] : nullptr;
}

////

int32 UCubismModelComponent::GetParameterCount() const
{
	return csmGetParameterCount(RawModel);
}

FString UCubismModelComponent::GetParameterId(const int32 ParameterIndex) const
{
	if (NonNativeParameterIds.Contains(ParameterIndex))
	{
		return NonNativeParameterIds[ParameterIndex];
	}

	check(0 <= ParameterIndex && ParameterIndex < GetParameterCount());

	const char* ParameterId = csmGetParameterIds(RawModel)[ParameterIndex];

	return FString(UTF8_TO_TCHAR(ParameterId));
}

int32 UCubismModelComponent::GetParameterIndex(const FString ParameterId)
{
	if (ParameterId.IsEmpty())
	{
		return -1;
	}

	if (!ParameterIndices.Contains(ParameterId))	
	{
		AddParameter(ParameterId);
	}

	if (!ParameterIndices.Contains(ParameterId))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetParameterIndex: Failed to add parameter '%s'."), *ParameterId);
		return -1;
	}

	return ParameterIndices[ParameterId];
}

UCubismParameterComponent* UCubismModelComponent::GetParameter(const int32 ParameterIndex) const
{
	return Parameters.IsValidIndex(ParameterIndex)? Parameters[ParameterIndex] : nullptr;
}

UCubismParameterComponent* UCubismModelComponent::GetParameter(const FString ParameterId)
{
	int32 ParameterIndex = GetParameterIndex(ParameterId);

	return GetParameter(ParameterIndex);
}

////

int32 UCubismModelComponent::GetPartCount() const
{
	return csmGetPartCount(RawModel);
}

FString UCubismModelComponent::GetPartId(const int32 PartIndex) const
{
	if (NonNativePartIds.Contains(PartIndex))
	{
		return NonNativePartIds[PartIndex];
	}

	check(0 <= PartIndex && PartIndex < GetPartCount());

	const char* PartId = csmGetPartIds(RawModel)[PartIndex];

	return FString(UTF8_TO_TCHAR(PartId));
}

int32 UCubismModelComponent::GetPartIndex(const FString PartId)
{
	if (PartId.IsEmpty())
	{
		return -1;
	}

	if (!PartIndices.Contains(PartId))
	{
		AddPart(PartId);
	}

	return PartIndices[PartId];
}

UCubismPartComponent* UCubismModelComponent::GetPart(const int32 PartIndex) const
{
	return Parts.IsValidIndex(PartIndex)? Parts[PartIndex] : nullptr;
}

UCubismPartComponent* UCubismModelComponent::GetPart(const FString PartId)
{
	int32 PartIndex = GetPartIndex(PartId);

	return GetPart(PartIndex);
}

////

ECubismParameterType UCubismModelComponent::GetParameterType(const int32 ParameterIndex) const
{
	if (NonNativeParameterIds.Contains(ParameterIndex))
	{
		return ECubismParameterType::Normal;
	}
	
	return (ECubismParameterType) csmGetParameterTypes(RawModel)[ParameterIndex];
}

float UCubismModelComponent::GetParameterMinimumValue(const int32 ParameterIndex) const
{
	if (NonNativeParameterIds.Contains(ParameterIndex))
	{
		return std::numeric_limits<float>::quiet_NaN();
	}

	return csmGetParameterMinimumValues(RawModel)[ParameterIndex];
}

float UCubismModelComponent::GetParameterMaximumValue(const int32 ParameterIndex) const
{
	if (NonNativeParameterIds.Contains(ParameterIndex))
	{
		return std::numeric_limits<float>::quiet_NaN();
	}

	return csmGetParameterMaximumValues(RawModel)[ParameterIndex];
}

float UCubismModelComponent::GetParameterDefaultValue(const int32 ParameterIndex) const
{
	if (NonNativeParameterIds.Contains(ParameterIndex))
	{
		return 0.0f;
	}

	return csmGetParameterDefaultValues(RawModel)[ParameterIndex];
}

float UCubismModelComponent::GetParameterValue(const int32 ParameterIndex) const
{
	if (NonNativeParameterValues.Contains(ParameterIndex))
	{
		return NonNativeParameterValues[ParameterIndex];
	}

	check(0 <= ParameterIndex && ParameterIndex < GetParameterCount());

	return csmGetParameterValues(RawModel)[ParameterIndex];
}

void UCubismModelComponent::SetParameterValue(const int32 ParameterIndex, const float Value)
{
	if (NonNativeParameterValues.Contains(ParameterIndex))
	{
		NonNativeParameterValues[ParameterIndex] = Value;
		return;
	}

	check(0 <= ParameterIndex && ParameterIndex < GetParameterCount());

	csmGetParameterValues(RawModel)[ParameterIndex] = Value;
}

int32 UCubismModelComponent::GetParameterKeyCount(const int32 ParameterIndex) const
{
	return csmGetParameterKeyCounts(RawModel)[ParameterIndex];
}

const float* UCubismModelComponent::GetParameterKeyValue(const int32 ParameterIndex) const
{
	return csmGetParameterKeyValues(RawModel)[ParameterIndex];
}

void UCubismModelComponent::AddParameter(const FString ParameterId)
{
	int32 ParameterIndex = GetParameterCount() + NonNativeParameterIds.Num();

	ParameterIndices.Add(ParameterId, ParameterIndex);
	NonNativeParameterIds.Add(ParameterIndex, ParameterId);
	NonNativeParameterValues.Add(ParameterIndex, 0.0f);

	const TObjectPtr<UCubismParameterComponent>& Parameter = NewObject<UCubismParameterComponent>(this, NAME_None, RF_Transactional);

	Parameter->Index = ParameterIndex;

	Parameter->RegisterComponent();

	Parameters.Add(Parameter);
}

////

float UCubismModelComponent::GetPartOpacity(const int32 PartIndex) const
{
	if (NonNativePartOpacities.Contains(PartIndex))
	{
		return NonNativePartOpacities[PartIndex];
	}

	check(0 <= PartIndex && PartIndex < csmGetPartCount(RawModel));

	return csmGetPartOpacities(RawModel)[PartIndex];
}

void UCubismModelComponent::SetPartOpacity(const int32 PartIndex, const float InOpacity)
{
	if (NonNativePartOpacities.Contains(PartIndex))
	{
		NonNativePartOpacities[PartIndex] = InOpacity;
		return;
	}

	check(0 <= PartIndex && PartIndex < csmGetPartCount(RawModel));

	csmGetPartOpacities(RawModel)[PartIndex] = InOpacity;
}

int UCubismModelComponent::GetPartParentPartIndex(const int32 PartIndex) const
{
	return csmGetPartParentPartIndices(RawModel)[PartIndex];
}

void UCubismModelComponent::AddPart(const FString PartId)
{
	int32 PartIndex = GetPartCount() + NonNativePartIds.Num();

	PartIndices.Add(PartId, PartIndex);
	NonNativePartIds.Add(PartIndex, PartId);
	NonNativePartOpacities.Add(PartIndex, 0.0f);

	const TObjectPtr<UCubismPartComponent>& Part = NewObject<UCubismPartComponent>(this, NAME_None, RF_Transactional);

	Part->Index = PartIndex;

	Part->RegisterComponent();

	Parts.Add(Part);
}

////

ECubismDrawableBlendMode UCubismModelComponent::GetDrawableBlendMode(const int32 DrawableIndex) const
{
	const csmFlags ConstantFlags = csmGetDrawableConstantFlags(RawModel)[DrawableIndex];

	if ((ConstantFlags & csmBlendAdditive) == csmBlendAdditive)
	{
		return ECubismDrawableBlendMode::Additive;
	}
	else if ((ConstantFlags & csmBlendMultiplicative) == csmBlendMultiplicative)
	{
		return ECubismDrawableBlendMode::Multiplicative;
	}
	else
	{
		return ECubismDrawableBlendMode::Normal;
	}
}

bool UCubismModelComponent::GetDrawableIsTwoSided(const int32 DrawableIndex) const
{
	const csmFlags ConstantFlags = csmGetDrawableConstantFlags(RawModel)[DrawableIndex];

	return (ConstantFlags & csmIsDoubleSided) == csmIsDoubleSided;
}

bool UCubismModelComponent::GetDrawableInvertedMask(const int32 DrawableIndex) const
{
	const csmFlags ConstantFlags = csmGetDrawableConstantFlags(RawModel)[DrawableIndex];

	return (ConstantFlags & csmIsInvertedMask) == csmIsInvertedMask;
}

bool UCubismModelComponent::GetDrawableDynamicFlagIsVisible(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmIsVisible) == csmIsVisible;
}

bool UCubismModelComponent::GetDrawableDynamicFlagVisibilityDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmVisibilityDidChange) == csmVisibilityDidChange;
}

bool UCubismModelComponent::GetDrawableDynamicFlagOpacityDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmOpacityDidChange) == csmOpacityDidChange;
}

bool UCubismModelComponent::GetDrawableDynamicFlagDrawOrderDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmDrawOrderDidChange) == csmDrawOrderDidChange;
}

bool UCubismModelComponent::GetDrawableDynamicFlagRenderOrderDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmRenderOrderDidChange) == csmRenderOrderDidChange;
}

bool UCubismModelComponent::GetDrawableDynamicFlagVertexPositionsDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmVertexPositionsDidChange) == csmVertexPositionsDidChange;
}

bool UCubismModelComponent::GetDrawableDynamicFlagBlendColorDidChange(const int32 DrawableIndex) const
{
	const csmFlags DynamicFlags = csmGetDrawableDynamicFlags(RawModel)[DrawableIndex];

	return (DynamicFlags & csmBlendColorDidChange) == csmBlendColorDidChange;
}

int32 UCubismModelComponent::GetDrawableTextureIndex(const int32 DrawableIndex) const
{
	return csmGetDrawableTextureIndices(RawModel)[DrawableIndex];
}

int32 UCubismModelComponent::GetDrawableDrawOrder(const int32 DrawableIndex) const
{
	return csmGetDrawableDrawOrders(RawModel)[DrawableIndex];
}

int32 UCubismModelComponent::GetDrawableRenderOrder(const int32 DrawableIndex) const
{
	return csmGetDrawableRenderOrders(RawModel)[DrawableIndex];
}

float UCubismModelComponent::GetDrawableOpacity(const int32 DrawableIndex) const
{
	return csmGetDrawableOpacities(RawModel)[DrawableIndex];
}

int32 UCubismModelComponent::GetDrawableMaskCount(const int32 DrawableIndex) const
{
	return csmGetDrawableMaskCounts(RawModel)[DrawableIndex];
}

const int32* UCubismModelComponent::GetDrawableMask(const int32 DrawableIndex) const
{
	return csmGetDrawableMasks(RawModel)[DrawableIndex];
}

int32 UCubismModelComponent::GetDrawableVertexCount(const int32 DrawableIndex) const
{
	return csmGetDrawableVertexCounts(RawModel)[DrawableIndex];
}

const csmVector2* UCubismModelComponent::GetDrawableVertexPosition(const int32 DrawableIndex) const
{
	return csmGetDrawableVertexPositions(RawModel)[DrawableIndex];
}

const csmVector2* UCubismModelComponent::GetDrawableVertexUv(const int32 DrawableIndex) const
{
	return csmGetDrawableVertexUvs(RawModel)[DrawableIndex];
}

int32 UCubismModelComponent::GetDrawableVertexIndexCount(const int32 DrawableIndex) const
{
	return csmGetDrawableIndexCounts(RawModel)[DrawableIndex];
}

const uint16* UCubismModelComponent::GetDrawableVertexIndex(const int32 DrawableIndex) const
{
	return reinterpret_cast<const uint16*>(csmGetDrawableIndices(RawModel)[DrawableIndex]);
}

FLinearColor UCubismModelComponent::GetDrawableMultiplyColor(const int32 DrawableIndex) const
{
	const csmVector4 Color = csmGetDrawableMultiplyColors(RawModel)[DrawableIndex];
	return FLinearColor(Color.X, Color.Y, Color.Z, Color.W);
}

FLinearColor UCubismModelComponent::GetDrawableScreenColor(const int32 DrawableIndex) const
{
	const csmVector4 Color = csmGetDrawableScreenColors(RawModel)[DrawableIndex];
	return FLinearColor(Color.X, Color.Y, Color.Z, Color.W);
}

int32 UCubismModelComponent::GetDrawableParentPartIndex(const int32 DrawableIndex) const
{
	return csmGetDrawableParentPartIndices(RawModel)[DrawableIndex];
}
////

// UObject interface
void UCubismModelComponent::PostLoad()
{
	Super::PostLoad();

	Setup();
}

#if WITH_EDITOR
void UCubismModelComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismModelComponent, bRenderInWorldSpace))
	{
		SetVisibility(bRenderInWorldSpace, true);
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismModelComponent, UserDataJson))
	{
		if (!UserDataJson)
		{
			for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Drawables)
			{
				Drawable->UserDataTag = TEXT("");
			}
		}
		else
		{
			const FCubismUserDataEntry& UserDataEntry = UserDataJson->Data[ECubismUserDataTargetType::ArtMesh];

			for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Drawables)
			{
				const FString Id = Drawable->Id;

				if (UserDataEntry.Tags.Contains(Id))
				{
					Drawable->UserDataTag = UserDataEntry.Tags[Id];
				}
				else
				{
					Drawable->UserDataTag = TEXT("");
				}
			}
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismModelComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	check(Moc);

	Moc->SetupModel(this);

	{
		const int32 DrawableCount = csmGetDrawableCount(RawModel);
		Drawables.Reserve(DrawableCount);

		for (int32 DrawableIndex = 0; DrawableIndex < DrawableCount; DrawableIndex++)
		{
			const FString DrawableId = GetDrawableId(DrawableIndex);
			DrawableIndices.Add(DrawableId, DrawableIndex);

			const TObjectPtr<UCubismDrawableComponent>& Drawable = NewObject<UCubismDrawableComponent>(this, *DrawableId, RF_Transactional);

			Drawable->Index = DrawableIndex;

			Drawable->RegisterComponent();
			GetOwner()->AddInstanceComponent(Drawable);
			Drawable->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

			Drawables.Add(Drawable);
		}
	}

	{
		const int32 ParameterCount = csmGetParameterCount(RawModel);
		Parameters.Reserve(ParameterCount);

		for (int32 ParameterIndex = 0; ParameterIndex < ParameterCount; ParameterIndex++)
		{
			const FString ParameterId = GetParameterId(ParameterIndex);
			ParameterIndices.Add(ParameterId, ParameterIndex);

			const TObjectPtr<UCubismParameterComponent>& Parameter = NewObject<UCubismParameterComponent>(this, *ParameterId, RF_Transactional);

			Parameter->Index = ParameterIndex;

			Parameter->RegisterComponent();

			Parameters.Add(Parameter);
		}
	}

	{
		const int32 PartCount = csmGetPartCount(RawModel);
		Parts.Reserve(PartCount);

		for (int32 PartIndex = 0; PartIndex < PartCount; PartIndex++)
		{
			const FString PartId = GetPartId(PartIndex);
			PartIndices.Add(PartId, PartIndex);

			const TObjectPtr<UCubismPartComponent>& Part = NewObject<UCubismPartComponent>(this, *PartId, RF_Transactional);

			Part->Index = PartIndex;

			Part->RegisterComponent();

			Parts.Add(Part);
		}
	}

	SetVisibility(bRenderInWorldSpace, true);
	if (GetOwner() && !GetOwner()->FindComponentByClass<UCubismUpdateControllerComponent>())
	{
		UCubismUpdateControllerComponent* Controller = NewObject<UCubismUpdateControllerComponent>(GetOwner(), UCubismUpdateControllerComponent::StaticClass(), TEXT("CubismUpdateController"));
		Controller->RegisterComponent();
		GetOwner()->AddInstanceComponent(Controller);
	}
}

void UCubismModelComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	for (const TObjectPtr<UCubismDrawableComponent>& Drawable : Drawables)
	{
		Drawable->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		GetOwner()->RemoveInstanceComponent(Drawable);
		Drawable->DestroyComponent();
	}

	Drawables.Empty();
	Parameters.Empty();
	Parts.Empty();

	DrawableIndices.Empty();
	ParameterIndices.Empty();
	PartIndices.Empty();

	Moc->DeleteModel(this);

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismModelComponent::PostEditUndo()
{
	Super::PostEditUndo();

	Setup();
}
#endif

void UCubismModelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	OnCubismUpdate(DeltaTime);
}

void UCubismModelComponent::OnCubismUpdate(float DeltaTime)
{
	if (RawModel)
	{
		csmUpdateModel(RawModel);

		csmResetDrawableDynamicFlags(RawModel);
	}
}

int32 UCubismModelComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_MODEL;
}
// End of UActorComponent interface
