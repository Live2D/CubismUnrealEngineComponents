/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Physics/CubismPhysicsComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Physics/CubismPhysicsRig.h"
#include "Physics/CubismPhysics3Json.h"
#include "CubismMath.h"
#include "Live2DCubismCore.h"

const float AirResistance = 5.0f;
const float MaximumWeight = 100.0f;
const float MovementThreshold = 0.001f;
const float MaxDeltaTime = 5.0f;

UCubismPhysicsComponent::UCubismPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismPhysicsComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismPhysicsComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;

		const int32 ParameterCount = Model->GetParameterCount();

		if (ParameterCaches.Num() != ParameterCount)
		{
			ParameterCaches.SetNum(ParameterCount);
		}

		if (ParameterInputCaches.Num() != ParameterCount)
		{
			ParameterInputCaches.SetNum(ParameterCount);
		}

		for (int32 ParameterIndex = 0; ParameterIndex < ParameterCount; ++ParameterIndex)
		{
			const UCubismParameterComponent* Parameter = Model->GetParameter(ParameterIndex);

			ParameterCaches[ParameterIndex] = Parameter->Value;
			ParameterInputCaches[ParameterIndex] = Parameter->Value;
		}
	}

	if (Json)
	{
		Rigs.Empty();

		Gravity = Json->Gravity;
		Wind = Json->Wind;
		Fps = Json->Fps;

		for (const FCubismPhysicsSetting& Setting : Json->PhysicsSettings)
		{
			FCubismPhysicsRig Rig;

			Rig.NormalizationPosition = Setting.NormalizationPosition;
			Rig.NormalizationAngle = Setting.NormalizationAngle;

			for (const FCubismPhysicsInput& Input : Setting.Inputs)
			{
				FCubismPhysicsRigInput RigInput;

				RigInput.ParameterIndex = Model->GetParameterIndex(Input.Source.Id);
				RigInput.Parameter = Model->GetParameter(RigInput.ParameterIndex);
				RigInput.Weight = Input.Weight / MaximumWeight;
				RigInput.bReflect = Input.bReflect;
				RigInput.Type = Input.Type;
				RigInput.Source = Input.Source;

				Rig.Inputs.Add(RigInput);
			}

			for (const FCubismPhysicsOutput& Output : Setting.Outputs)
			{
				FCubismPhysicsRigOutput RigOutput;

				RigOutput.ParameterIndex = Model->GetParameterIndex(Output.Destination.Id);
				RigOutput.Parameter = Model->GetParameter(RigOutput.ParameterIndex);
				RigOutput.ParticleIndex = Output.VertexIndex;
				RigOutput.AngleScale = Output.AngleScale;
				RigOutput.Weight = Output.Weight / MaximumWeight;
				RigOutput.bReflect = Output.bReflect;
				RigOutput.Type = Output.Type;
				RigOutput.Destination = Output.Destination;

				Rig.Outputs.Add(RigOutput);
			}

			for (const FCubismPhysicsParticle& Particle : Setting.Particles)
			{
				FCubismPhysicsRigParticle RigParticle;

				RigParticle.Mobility = Particle.Mobility;
				RigParticle.Delay = Particle.Delay;
				RigParticle.Acceleration = Particle.Acceleration;
				RigParticle.Radius = Particle.Radius;
				RigParticle.Position = Particle.Position;

				Rig.Particles.Add(RigParticle);
			}

			Rigs.Add(Rig);
		}

		Initialize();
	}

	if (Model->Physics != this)
	{
		if (Model->Physics)
		{
			Model->Physics->DestroyComponent();
		}
		Model->Physics = this;
	}

	Model->AddTickPrerequisiteComponent(this); // model ticks after parameters are updated by components
}

void UCubismPhysicsComponent::Initialize()
{
	for (FCubismPhysicsRig& Rig : Rigs)
	{
		for (int32 ParticleIndex = 0; ParticleIndex < Rig.Particles.Num(); ++ParticleIndex)
		{
			FCubismPhysicsRigParticle& Particle = Rig.Particles[ParticleIndex];

			Particle.InitialPosition.X = ParticleIndex == 0? 0.0f : Rig.Particles[ParticleIndex-1].InitialPosition.X;
			Particle.InitialPosition.Y = ParticleIndex == 0? 0.0f : Rig.Particles[ParticleIndex-1].InitialPosition.Y + Particle.Radius;

			Particle.Position = Particle.InitialPosition;
			Particle.LastPosition = Particle.InitialPosition;
			Particle.LastGravity = FVector2D(0.0f, 1.0f);
			Particle.Velocity = FVector2D::ZeroVector;
		}
	}
}

void UCubismPhysicsComponent::Stabilization()
{
	for (FCubismPhysicsRig& Rig : Rigs)
	{
		FVector2D TotalTranslation = FVector2D::ZeroVector;
		float TotalAngle = 0.0f;

		for (FCubismPhysicsRigInput& Input : Rig.Inputs)
		{
			const float Value = Input.Parameter->Value;

			Input.GetNormalizedParameterValue(TotalTranslation, TotalAngle, Value, Rig.NormalizationPosition, Rig.NormalizationAngle);

			ParameterCaches[Input.ParameterIndex] = Value;
		}

		const float RadAngle = -TotalAngle * (PI / 180.0f);

		TotalTranslation.X = FMath::Cos(RadAngle) * TotalTranslation.X - FMath::Sin(RadAngle) * TotalTranslation.Y;
		TotalTranslation.Y = FMath::Sin(RadAngle) * TotalTranslation.X + FMath::Cos(RadAngle) * TotalTranslation.Y;

		UpdateParticlesForStabilization(
			Rig.Particles,
			TotalTranslation,
			TotalAngle,
			MovementThreshold * Rig.NormalizationPosition.Maximum
		);

		for (FCubismPhysicsRigOutput& Output : Rig.Outputs)
		{
			if (1 <= Output.ParticleIndex && Output.ParticleIndex < Rig.Particles.Num())
			{
				const float OutputValue = Output.GetValue(Rig.Particles, Gravity);

				Output.PreviousValue = OutputValue;
				Output.CurrentValue = OutputValue;

				float TargetValue = Output.Parameter->Value;

				Output.UpdateOutputParameterValue(TargetValue, OutputValue);

				Output.Parameter->SetParameterValue(TargetValue);

				ParameterCaches[Output.ParameterIndex] = TargetValue;
			}
		}
	}
}

void UCubismPhysicsComponent::UpdateParticles(
	TArray<FCubismPhysicsRigParticle>& Strand,
	const FVector2D TotalTranslation,
	const float TotalAngle,
	const float ThresholdValue,
	const float DeltaTime,
	const float Resistance
)
{
	Strand[0].Position.X = TotalTranslation.X;
	Strand[0].Position.Y = TotalTranslation.Y;

	const float TotalRadian = (TotalAngle / 180.0f)* PI;

	const FVector2D CurrentGravity = FVector2D(FMath::Sin(TotalRadian), FMath::Cos(TotalRadian));

	for (int32 i = 1; i < Strand.Num(); ++i)
	{
		Strand[i].Force = CurrentGravity * Strand[i].Acceleration + Wind;
		Strand[i].LastPosition = Strand[i].Position;

		// The Cubism Editor expects 30 FPS so we scale here by 30.
		const float Delay = Strand[i].Delay * DeltaTime * 30.0f;

		FVector2D Direction = Strand[i].Position - Strand[i - 1].Position;
	
		const float Radian = FCubismMath::DirectionToRadian(Strand[i].LastGravity, CurrentGravity) / Resistance;
		Direction.X = (FMath::Cos(Radian) * Direction.X) - (Direction.Y * FMath::Sin(Radian));
		Direction.Y = (FMath::Sin(Radian) * Direction.X) + (Direction.Y * FMath::Cos(Radian));

		Strand[i].Position = Strand[i - 1].Position + Direction;

		const FVector2D Velocity = Strand[i].Velocity * Delay;
		const FVector2D Force = Strand[i].Force * Delay * Delay;

		Strand[i].Position = Strand[i].Position + Velocity + Force;

		FVector2D NewDirection = Strand[i].Position - Strand[i - 1].Position;

		NewDirection.Normalize();

		Strand[i].Position = Strand[i - 1].Position + (NewDirection * Strand[i].Radius);

		Direction = Strand[i].Radius * Direction.GetSafeNormal();

		if (FMath::Abs(Strand[i].Position.X) < ThresholdValue)
		{
			Strand[i].Position.X = 0.0f;
		}

		if (Delay != 0.0f)
		{
			Strand[i].Velocity = (Strand[i].Position - Strand[i].LastPosition);
			Strand[i].Velocity /= Delay;
			Strand[i].Velocity *= Strand[i].Mobility;
		}

		Strand[i].Force = FVector2D(0.0f, 0.0f);
		Strand[i].LastGravity = CurrentGravity;
	}
}

void UCubismPhysicsComponent::UpdateParticlesForStabilization(
	TArray<FCubismPhysicsRigParticle>& Particles,
	const FVector2D TotalTranslation,
	const float TotalAngle,
	const float ThresholdValue
)
{
	Particles[0].Position.X = TotalTranslation.X;
	Particles[0].Position.Y = TotalTranslation.Y;

	const float TotalRadian = TotalAngle * (PI / 180.0f);

	const FVector2D CurrentGravity = FVector2D(FMath::Sin(TotalRadian), FMath::Cos(TotalRadian));

	for (int32 ParticleIndex = 1; ParticleIndex < Particles.Num(); ++ParticleIndex)
	{
		FCubismPhysicsRigParticle& Particle = Particles[ParticleIndex];

		FVector2D NewPosition = CurrentGravity * Particle.Acceleration + Wind;
		NewPosition = Particle.Radius * NewPosition.GetSafeNormal();

		Particle.LastPosition = Particle.Position;
		Particle.Position = NewPosition + Particles[ParticleIndex - 1].Position;

		Particle.Velocity = FVector2D::ZeroVector;

		if (FMath::Abs(Particle.Position.X) < ThresholdValue)
		{
			Particle.Position.X = 0.0f;
		}

		Particle.LastGravity = CurrentGravity;
	}
}

// UObject interface
void UCubismPhysicsComponent::PostLoad()
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
void UCubismPhysicsComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismPhysicsComponent, Json))
	{
		Setup(Model);
	}

	const FName EnablePhysicsPropertyName = PropertyChangedEvent.GetPropertyName();

	if (EnablePhysicsPropertyName == GET_MEMBER_NAME_CHECKED(UCubismPhysicsComponent, bEnablePhysicsInEditor))
	{
		if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
		{
			bTickInEditor = bEnablePhysicsInEditor;
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismPhysicsComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		SetComponentTickEnabled(bEnablePhysicsInEditor);
	}
#endif
}

void UCubismPhysicsComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model && Model->Physics == this)
	{
		Model->Physics = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismPhysicsComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	OnCubismUpdate(DeltaTime);
}

void UCubismPhysicsComponent::OnCubismUpdate(float DeltaTime)
{
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor && !bEnablePhysicsInEditor)
	{
		return;
	}
#endif

	if (!Model)
	{
		return;
	}

	CurrentRemainTime += DeltaTime;

	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("Model is null."));
		return;
	}

	if (CurrentRemainTime > MaxDeltaTime)
	{
		CurrentRemainTime = 0.0f;
	}

	const int32 ParameterCount = Model->GetParameterCount();

	if (ParameterCaches.Num() < ParameterCount)
	{
		ParameterCaches.SetNum(ParameterCount);
	}
	if (ParameterInputCaches.Num() < ParameterCount)
	{
		ParameterInputCaches.SetNum(ParameterCount);
		for (int j = 0; j < ParameterCount; ++j) {
			ParameterInputCaches[j] = ParameterCaches[j];
		}
	}

	const float PhysicsDeltaTime = Fps > 0.0f? 1.0f / Fps : DeltaTime;

	while (CurrentRemainTime >= PhysicsDeltaTime)
	{
		const float InputWeight = PhysicsDeltaTime / CurrentRemainTime;

		check(InputWeight >= 0.0f && InputWeight <= 1.0f);

		for (int32 ParameterIndex = 0; ParameterIndex < ParameterCount; ++ParameterIndex)
		{
			if (const UCubismParameterComponent* Parameter = Model->GetParameter(ParameterIndex))
			{
				ParameterCaches[ParameterIndex] = ParameterInputCaches[ParameterIndex] * (1.0f - InputWeight) + Parameter->Value * InputWeight;
				ParameterInputCaches[ParameterIndex] = ParameterCaches[ParameterIndex];
			}
		}

		// update each pendulum
		for (FCubismPhysicsRig& Rig : Rigs)
		{
			FVector2D TotalTranslation = FVector2D::ZeroVector;
			float TotalAngle = 0.0f;

			for (FCubismPhysicsRigInput& Input : Rig.Inputs)
			{
				const float Value = ParameterCaches[Input.ParameterIndex];

				Input.GetNormalizedParameterValue(TotalTranslation, TotalAngle, Value, Rig.NormalizationPosition, Rig.NormalizationAngle);
			}

			const float RadAngle = -TotalAngle * (PI / 180.0f);

			TotalTranslation.X = FMath::Cos(RadAngle) * TotalTranslation.X - FMath::Sin(RadAngle) * TotalTranslation.Y;
			TotalTranslation.Y = FMath::Sin(RadAngle) * TotalTranslation.X + FMath::Cos(RadAngle) * TotalTranslation.Y;

			UpdateParticles(
				Rig.Particles,
				TotalTranslation,
				TotalAngle,
				MovementThreshold * Rig.NormalizationPosition.Maximum,
				PhysicsDeltaTime,
				AirResistance
			);

			for (FCubismPhysicsRigOutput& Output : Rig.Outputs)
			{
				const float OutputValue = Output.GetValue(Rig.Particles, Gravity);

				Output.PreviousValue = Output.CurrentValue;
				Output.CurrentValue = OutputValue;

				Output.UpdateOutputParameterValue(ParameterCaches[Output.ParameterIndex], OutputValue);
			}
		}

		CurrentRemainTime -= PhysicsDeltaTime;
	}

	const float Weight = CurrentRemainTime / PhysicsDeltaTime;

	check(Weight >= 0.0f && Weight <= 1.0f);

	for (FCubismPhysicsRig& Rig : Rigs)
	{
		for (FCubismPhysicsRigOutput& Output : Rig.Outputs)
		{
			if (Output.ParameterIndex < 0)
			{
				continue;
			}

			const float OutputValue = Output.PreviousValue * (1.0f - Weight) + Output.CurrentValue * Weight;
			float TargetValue = Output.Parameter->Value;

			Output.UpdateOutputParameterValue(TargetValue, OutputValue);

			Output.Parameter->SetParameterValue(TargetValue);
		}
	}
}

int32 UCubismPhysicsComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_PHYSICS;
}
// End of UActorComponent interface
