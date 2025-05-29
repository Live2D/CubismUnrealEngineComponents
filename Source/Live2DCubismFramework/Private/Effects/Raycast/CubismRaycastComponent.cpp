/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/Raycast/CubismRaycastComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Effects/Raycast/CubismRaycastParameter.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismDrawableComponent.h"
#include "Model/CubismModel3Json.h"

UCubismRaycastComponent::UCubismRaycastComponent()
{
}

void UCubismRaycastComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismRaycastComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	if (Json)
	{
		Parameters.Empty();

		for (const FHitAreaEntry& Entry : Json->HitAreas)
		{
			FCubismRaycastParameter Parameter;

			Parameter.bEnabled = true;
			Parameter.Id = Entry.Id;
			Parameter.Name = Entry.Name;

			Parameters.Add(Parameter);
		}
	}

	if (Model->Raycast != this)
	{
		if (Model->Raycast)
		{
			Model->Raycast->DestroyComponent();
		}
		Model->Raycast = this;
	}
}

void UCubismRaycastComponent::Raycast(
	const FVector Origin,
	const FVector Direction,
	TArray<FCubismRaycastHit>& Result,
	const float Length
) const
{
	Result.Empty();

	const FVector NormDir = Direction.GetSafeNormal();

	for (const FCubismRaycastParameter& Parameter : Parameters)
	{
		if (!Parameter.bEnabled)
		{
			continue;
		}

		UCubismDrawableComponent* Drawable = Model->GetDrawable(Parameter.Id);

		if (!Drawable)
		{
			continue;
		}

		const FTransform& Transform = Drawable->GetComponentTransform();
		FVector HitPosition, HitNormal;
		float HitTime;

		if(RaycastDrawable(
			Origin, NormDir, Length,
			Transform, Parameter.Precision,
			Drawable,
			HitPosition, HitNormal, HitTime
		))
		{
			FCubismRaycastHit RaycastHit;

			RaycastHit.Drawable = Drawable;
			RaycastHit.Distance = HitTime * Length;
			RaycastHit.GlobalPosition = HitPosition;
			RaycastHit.LocalPosition = Transform.InverseTransformPosition(HitPosition);

			Result.Add(RaycastHit);
		}
	}
}

bool UCubismRaycastComponent::RaycastDrawable(
	const FVector Origin, const FVector Dir, const float Length,
	const FTransform& Transform,
	const ECubismRaycastPrecision& Precision,
	const UCubismDrawableComponent* Drawable,
	FVector& HitPosition, FVector& HitNormal, float& HitTime
) const
{
	const FBox Box = Drawable->CalcBounds(Transform).GetBox();
	const FVector End = Origin + Dir * Length;

	if (!FMath::LineExtentBoxIntersection(
		Box,
		Origin, End, FVector::Zero(),
		HitPosition, HitNormal, HitTime
	))
	{
		return false;
	}

	switch (Precision)
	{
		case ECubismRaycastPrecision::BoundingBox:
		{
			// already checked

			break;
		}
		case ECubismRaycastPrecision::Mesh:
		{
			const TArray<int32> Indices = Drawable->GetVertexIndices();

			TArray<FVector> Positions;
			for (const FVector2D& Position : Drawable->GetVertexPositions())
			{
				Positions.Add(Model->GetRelativeTransform().TransformPosition(Drawable->ToGlobalPosition(Position)));
			}

			if (!RayIntersectMesh(
				Origin, Dir, Length,
				Positions, Indices,
				HitPosition, HitTime
			))
			{
				return false;
			}

			break;
		}
		default:
		{
			ensure(false);
			break;
		}
	}

	return true;
}

bool UCubismRaycastComponent::RayIntersectMesh
(
	const FVector Origin, const FVector Dir, const float Length,
	const TArray<FVector> Positions, const TArray<int32> Indices,
	FVector& HitPosition, float& HitTime
)
{
	for (int32 i = 0; i < Indices.Num(); i += 3)
	{
		const FVector T0 = Positions[Indices[i    ]];
		const FVector T1 = Positions[Indices[i + 1]];
		const FVector T2 = Positions[Indices[i + 2]];

		if (RayIntersectTriangle(Origin, Dir, Length, T0, T1, T2, HitPosition, HitTime))
		{
			return true;
		}
	}

	return false;
}

// Möller-Trumbore intersection algorithm
// https://www.graphics.cornell.edu/pubs/1997/MT97.pdf
bool UCubismRaycastComponent::RayIntersectTriangle
(
	const FVector Origin, const FVector Dir, const float Length,
	const FVector T0, const FVector T1, const FVector T2,
	FVector& HitPosition, float& HitTime
)
{
	const FVector E1 = T1 - T0;
	const FVector E2 = T2 - T0;

	const FVector P = Dir ^ E2;
	const float Det = E1 | P;

	if (FMath::IsNearlyZero(Det))
	{
		return false;
	}

	const float InvDet = 1.0f / Det;
	const FVector T = Origin - T0;
	const float U = (T | P) * InvDet;

	if (U < 0.0f || U > 1.0f)
	{
		return false;
	}

	const FVector Q = T ^ E1;
	const float V = (Dir | Q) * InvDet;

	if (V < 0.0f || U + V > 1.0f)
	{
		return false;
	}

	const float W = (E2 | Q) * InvDet;

	HitTime = W / Length;

	if (HitTime < 0.0f || HitTime > 1.0f)
	{
		return false;
	}

	HitPosition = Origin + Dir * W;

	return true;
}

// UObject interface
void UCubismRaycastComponent::PostLoad()
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
// End of UObject interface

// UActorComponent interface
void UCubismRaycastComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismRaycastComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model && Model->Raycast == this)
	{
		Model->Raycast = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismRaycastComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismRaycastComponent::OnCubismUpdate(float DeltaTime)
{
}

int32 UCubismRaycastComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_RAYCAST;
}
// End of UActorComponent interface
