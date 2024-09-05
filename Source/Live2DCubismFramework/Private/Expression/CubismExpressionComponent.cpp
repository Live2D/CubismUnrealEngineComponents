/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Expression/CubismExpressionComponent.h"

#include "Expression/CubismExpression.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "CubismLog.h"

UCubismExpressionComponent::UCubismExpressionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;
}

void UCubismExpressionComponent::Setup(UCubismModelComponent* InModel)
{
	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Time = 0.0f;
	ExpressionQueue.Empty();

	AddTickPrerequisiteComponent(Model->ParameterStore); // must be updated after parameters loaded
	AddTickPrerequisiteComponent(Model->Motion); // must be updated at first because motions overwrite parameters
	AddTickPrerequisiteComponent(Model->Pose); // must be updated at first because poses overwrite parameters
	Model->AddTickPrerequisiteComponent(this); // must update parameters on memory after parameter updated
}

void UCubismExpressionComponent::PlayExpression(const int32 InIndex)
{
	if (!Jsons.IsValidIndex(InIndex))
	{
		UE_LOG(LogCubism, Warning, TEXT("Expression cannot be played. Index is out of range."));

		return;
	}

	const TObjectPtr<UCubismExp3Json>& Json = Jsons[InIndex];

	for (const TSharedPtr<FCubismExpression>& Expression : ExpressionQueue)
	{
		Expression->StartFadeout(Time);
	}

	TSharedPtr<FCubismExpression> NextExpression = MakeShared<FCubismExpression>(Json);

	ExpressionQueue.Add(NextExpression);
}

void UCubismExpressionComponent::StopAllExpressions(const bool bForce)
{
	if (bForce)
	{
		ExpressionQueue.Empty();
	}
	else
	{
		for (const TSharedPtr<FCubismExpression>& Expression : ExpressionQueue)
		{
			Expression->StartFadeout(Time);
		}
	}
}

// UObject interface
void UCubismExpressionComponent::PostLoad()
{
	Super::PostLoad();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

#if WITH_EDITOR
void UCubismExpressionComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCubismExpressionComponent, Index))
	{
		if (Jsons.IsValidIndex(Index))
		{
			PlayExpression(Index);
		}
		else
		{
			StopAllExpressions();
		}
	}
}
#endif
// End of UObject interface

// UActorComponent interface
void UCubismExpressionComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}

void UCubismExpressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Time += DeltaTime;

	float ExpressionWeight = 0.0f;

	for (int32 ExpressionIndex = 0; ExpressionIndex < ExpressionQueue.Num(); ExpressionIndex++)
	{
		const TSharedPtr<FCubismExpression>& Expression = ExpressionQueue[ExpressionIndex];
			
		UpdateExpression(ExpressionIndex, Expression);

		ExpressionWeight += Expression->CalcExpressionWeight(Time);
	}

	if (ExpressionQueue.Num() > 1)
	{
		const TSharedPtr<FCubismExpression>& LatestExpression = ExpressionQueue.Last();

		if (LatestExpression->FadeWeight >= 1.0f)
		{
			for (int32 i = ExpressionQueue.Num()-2; i >= 0; i--)
			{
				ExpressionQueue.RemoveAt(i);
			}
		}
	}

	if (ExpressionQueue.Num() == 0)
	{
		OnExpressionPlaybackFinished.Broadcast();
	}

	const float Weight = FMath::Min(ExpressionWeight, 1.0f);

	for (FCubismExpressionParameterValue& ParameterValue : ParameterValues)
	{
		UCubismParameterComponent* DstParameter = Model->GetParameter(ParameterValue.Id);

		const float Value = (ParameterValue.OverwriteValue + ParameterValue.AdditiveValue) * ParameterValue.MultiplyValue;

		DstParameter->SetParameterValue(Value, Weight);

		ParameterValue.AdditiveValue = 0.0f;
		ParameterValue.MultiplyValue = 1.0f;
	}
}
// End of UActorComponent interface

void UCubismExpressionComponent::UpdateExpression(const int32 ExpressionIndex, const TSharedPtr<FCubismExpression>& Expression)
{
	// add new parameters if not exist
	for (int32 i = 0; i < Expression->Parameters.Num(); i++)
	{
		const FCubismExpressionParameter& ExpressionParameter = Expression->Parameters[i];
		const FString Id = ExpressionParameter.Id;

		bool bFound = false;
		for (const FCubismExpressionParameterValue& ParameterValue : ParameterValues)
		{
			if (Id == ParameterValue.Id)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			if (const UCubismParameterComponent* Parameter = Model->GetParameter(Id))
			{
				FCubismExpressionParameterValue ParameterValue = FCubismExpressionParameterValue();

				ParameterValue.Index = i;
				ParameterValue.Id = Id;
				ParameterValue.AdditiveValue = 0.0f;
				ParameterValue.MultiplyValue = 1.0f;
				ParameterValue.OverwriteValue = Parameter->GetParameterValue();

				ParameterValues.Add(ParameterValue);
			}
		}
	}

	if (Expression->State == ECubismExpressionState::None)
	{
		Expression->Init(Time);
	}

	const float ElapsedTime = Time - Expression->StartTime;
	const float FadeWeight = Expression->UpdateWeight(ElapsedTime);

	for (FCubismExpressionParameterValue& ParameterValue : ParameterValues)
	{
		if (!Expression->Parameters.IsValidIndex(ParameterValue.Index))
		{
			continue;
		}

		float NewAdditiveValue = 0.0f;
		float NewMultiplyValue = 1.0f;
		float NewOverwriteValue = Model->GetParameter(ParameterValue.Id)->GetParameterValue();

		const FCubismExpressionParameter& Parameter = Expression->Parameters[ParameterValue.Index];
		int32 ParameterIndex = -1;

		for (int32 i = 0; i < Expression->Parameters.Num(); ++i)
		{
			if (ParameterValue.Id != Expression->Parameters[i].Id)
			{
				continue;
			}
			ParameterIndex = i;

			break;
		}

		if (ParameterIndex < 0)
		{
			if (ExpressionIndex == 0)
			{
				ParameterValue.AdditiveValue = 0.0f;

				ParameterValue.MultiplyValue = 1.0f;

				ParameterValue.OverwriteValue = NewOverwriteValue;
			}
			else
			{
				ParameterValue.AdditiveValue =
					CalculateValue(ParameterValue.AdditiveValue, 0.0f, FadeWeight);

				ParameterValue.MultiplyValue =
					CalculateValue(ParameterValue.MultiplyValue, 1.0f, FadeWeight);

				ParameterValue.OverwriteValue =
					CalculateValue(ParameterValue.OverwriteValue, NewOverwriteValue, FadeWeight);
			}
			continue;
		}

		switch (Parameter.Blend)
		{
			case ECubismParameterBlendMode::Additive:
			{
				NewAdditiveValue = Parameter.Value;
				NewMultiplyValue = 1.0f;
				NewOverwriteValue = NewOverwriteValue;
				break;
			}
			case ECubismParameterBlendMode::Multiplicative:
			{
				NewAdditiveValue = 0.0f;
				NewMultiplyValue = Parameter.Value;
				NewOverwriteValue = NewOverwriteValue;
				break;
			}
			case ECubismParameterBlendMode::Overwrite:
			{
				NewAdditiveValue = 0.0f;
				NewMultiplyValue = 1.0f;
				NewOverwriteValue = Parameter.Value;
				break;
			}
			default:
			{
				ensure(false);
				break;
			}
		}

		if (ExpressionIndex == 0)
		{
			ParameterValue.AdditiveValue  = NewAdditiveValue;
			ParameterValue.MultiplyValue  = NewMultiplyValue;
			ParameterValue.OverwriteValue = NewOverwriteValue;
		}
		else
		{
			ParameterValue.AdditiveValue = (ParameterValue.AdditiveValue * (1.0f - FadeWeight)) + NewAdditiveValue * FadeWeight;
			ParameterValue.MultiplyValue = (ParameterValue.MultiplyValue * (1.0f - FadeWeight)) + NewMultiplyValue * FadeWeight;
			ParameterValue.OverwriteValue = (ParameterValue.OverwriteValue * (1.0f - FadeWeight)) + NewOverwriteValue * FadeWeight;
		}
	}
}

float UCubismExpressionComponent::CalculateValue(float Source, float Destination, float FadeWeight)
{
	return (Source * (1.0f - FadeWeight)) + (Destination * FadeWeight);
}
