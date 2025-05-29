/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Effects/LipSync/CubismLipSyncComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismModel3Json.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundBase.h"

const float FrameRate = 30.0f;
const float Epsilon = 0.01f;

UCubismLipSyncComponent::UCubismLipSyncComponent()
	: LipSyncTargetValue(0.0f)
	, LipSyncValue(0.0f)
	, LipSyncVValue(0.0f)
	, LastTimeSeconds(0.0f)
	, UserTimeSeconds(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismLipSyncComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCubismLipSyncComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Time = 0.0f;
	TargetValue = 0.0f;
	CurrentVelocity = 0.0f;

	if (Json)
	{
		Ids.Empty();

		Ids.Append(Json->LipSyncs);
	}

	if (!Audio)
	{
		Audio = CreateAudioComponent();
	}

	if (Model->LipSync != this)
	{
		if (Model->LipSync)
		{
			Model->LipSync->DestroyComponent();
		}
		Model->LipSync = this;
	}

	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

UAudioComponent* UCubismLipSyncComponent::GetAudioComponent()
{
	return Audio;
}

void UCubismLipSyncComponent::OnEnvelopeValue(const USoundWave* InSoundWave, const float InEnvelopeValue)
{
	TargetValue = FMath::Clamp(Gain * InEnvelopeValue, 0.0f, 1.0f);
}

TObjectPtr<UAudioComponent> UCubismLipSyncComponent::CreateAudioComponent()
{
	TObjectPtr<UAudioComponent> NewAudio = NewObject<UAudioComponent>(Model, NAME_None, RF_Transactional | RF_Transient);

	NewAudio->OnAudioSingleEnvelopeValue.AddUniqueDynamic(this, &UCubismLipSyncComponent::OnEnvelopeValue);

	if (!NewAudio->GetAttachParent() && !NewAudio->IsAttachedTo(Model))
	{
		AActor* Owner = GetOwner();

		if (!Owner || !Owner->GetWorld())
		{
			if (UWorld* World = GetWorld())
			{
				NewAudio->RegisterComponentWithWorld(World);
				NewAudio->AttachToComponent(Model, FAttachmentTransformRules::KeepRelativeTransform);
			}
			else
			{
				NewAudio->SetupAttachment(Model);
			}
		}
		else
		{
			NewAudio->AttachToComponent(Model, FAttachmentTransformRules::KeepRelativeTransform);
			NewAudio->RegisterComponent();
		}
	}

	NewAudio->bAutoActivate = !bAutoEnabled;
	NewAudio->bStopWhenOwnerDestroyed = true;
	NewAudio->bShouldRemainActiveIfDropped = true;
	NewAudio->Mobility = EComponentMobility::Movable;

	#if WITH_EDITORONLY_DATA
	NewAudio->bVisualizeComponent = false;
	#endif

	NewAudio->SetSound(Source);

	return NewAudio;
}

// UObject interface
void UCubismLipSyncComponent::PostLoad()
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
void UCubismLipSyncComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismLipSyncComponent, Value))
	{
		for (const FString& Id : Ids)
		{
			UCubismParameterComponent* Destination = Model->GetParameter(Id);

			if (!Destination)
			{
				continue;
			}

			switch (BlendMode)
			{
				case ECubismParameterBlendMode::Overwrite:
				{
					Destination->SetParameterValue(Value);
					break;
				}
				case ECubismParameterBlendMode::Additive:
				{
					Destination->AddParameterValue(Value);
					break;
				}
				case ECubismParameterBlendMode::Multiplicative:
				{
					Destination->MultiplyParameterValue(Value);
					break;
				}
				default:
				{
					ensure(false);
					break;
				}
			}
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismLipSyncComponent, bAutoEnabled))
	{
		Time = 0.0f;

		if (Audio)
		{
			Audio->bAutoActivate = !bAutoEnabled;
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismLipSyncComponent, Source))
	{
		if (Audio)
		{
			Audio->SetSound(Source);
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface.
void UCubismLipSyncComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismLipSyncComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	const AActor* Owner = GetOwner();

	if (Audio && !Audio->IsBeingDestroyed())
	{
		if (Owner && Owner->GetWorld())
		{
			Audio->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		}
		Audio->DestroyComponent();
		Audio = nullptr;
	}

	if (Model && Model->LipSync == this)
	{
		Model->LipSync = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismLipSyncComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismLipSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	OnCubismUpdate(DeltaTime);
}

void UCubismLipSyncComponent::OnCubismUpdate(float DeltaTime)
{
	if (!Model)
	{
		return;
	}

	Update(DeltaTime);

	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("LipSyncComponent Tick: Model is null."));
		return;
	}

	for (const FString& Id : Ids)
	{
		UCubismParameterComponent* Destination = Model->GetParameter(Id);

		if (!Destination)
		{
			continue;
		}

		switch (BlendMode)
		{
			case ECubismParameterBlendMode::Overwrite:
			{
				Destination->SetParameterValue(Value);
				break;
			}
			case ECubismParameterBlendMode::Additive:
			{
				Destination->AddParameterValue(Value);
				break;
			}
			case ECubismParameterBlendMode::Multiplicative:
			{
				Destination->MultiplyParameterValue(Value);
				break;
			}
			default:
			{
				ensure(false);
				break;
			}
		}
	}
}

int32 UCubismLipSyncComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_LIPSYNC;
}
// End of UActorComponent interface.

void UCubismLipSyncComponent::Update(const float DeltaTime)
{
	if (bAutoEnabled)
	{
		Time += DeltaTime;
		Value = FMath::Abs(FMath::Sin(TimeScale * Time));
	}
	else
	{
		Value = SmoothDamp(Value, DeltaTime);
	}
}

float UCubismLipSyncComponent::SmoothDamp(const float CurrentValue, const float DeltaTime)
{
	LipSyncTargetValue = TargetValue - CurrentValue;

	UserTimeSeconds += DeltaTime;

	const float FaceParamMaxV = 40.0f / 10.0f;
	const float MaxV = FaceParamMaxV * 1.0f / FrameRate;

	if (LastTimeSeconds == 0.0f)
	{
		LastTimeSeconds = UserTimeSeconds;
		return CurrentValue;
	}

	const float DeltaTimeWeight = (UserTimeSeconds - LastTimeSeconds) * FrameRate;
	LastTimeSeconds = UserTimeSeconds;

	const float TimeToMaxSpeed = 0.15f;
	const float FrameToMaxSpeed = TimeToMaxSpeed * FrameRate;     // sec * frame/sec
	const float MaxA = DeltaTimeWeight * MaxV / FrameToMaxSpeed;

	const float DX = LipSyncTargetValue - LipSyncValue;

	if (FMath::Abs(DX) <= Epsilon)
	{
		return CurrentValue;
	}

	const float D = FMath::Sqrt((DX * DX));

	const float VX = MaxV * DX / D;

	float AX = VX - LipSyncVValue;

	const float A = FMath::Sqrt((AX * AX));


	if (A < -MaxA || A > MaxA)
	{
		AX *= MaxA / A;
	}

	LipSyncVValue += AX;

	{
		const float MaxVelocity = 0.5f * (FMath::Sqrt((MaxA * MaxA) + 16.0f * MaxA * D - 8.0f * MaxA * D) - MaxA);
		const float CurVelocity = FMath::Sqrt(LipSyncVValue * LipSyncVValue);

		if (CurVelocity > MaxVelocity)
		{
			LipSyncVValue *= MaxVelocity / CurVelocity;
		}
	}

	LipSyncValue += LipSyncVValue;

	return LipSyncValue;
}
