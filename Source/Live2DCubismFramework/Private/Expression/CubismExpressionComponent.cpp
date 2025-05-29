/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Expression/CubismExpressionComponent.h"

#include "CubismUpdateExecutionOrder.h"
#include "Expression/CubismExpression.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"
#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "CubismLog.h"

UCubismExpressionComponent::UCubismExpressionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickInEditor = true;
}

void UCubismExpressionComponent::Setup(UCubismModelComponent* InModel)
{
	if (!InModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismExpressionComponent::Setup - InModel is null. Skipping setup."));
		return;
	}

	check(InModel);

	if (Model != InModel)
	{
		Model = InModel;
	}

	Time = 0.0f;
	ExpressionQueue.Empty();

	if (Model->Expression != this)
	{
		if (Model->Expression)
		{
			Model->Expression->DestroyComponent();
		}
		Model->Expression = this;
	}

	Model->AddTickPrerequisiteComponent(this); // model ticks after parameters are updated by components
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

	if (!Owner || !Owner->Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Owner or Model."));
		return;
	}
	Setup(Owner->Model);

	if (Index >= 0 && Jsons.IsValidIndex(Index))
	{
		PlayExpression(Index);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CubismExpressionComponent: Expression not assigned (index %d)"), Index);
	}
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

void UCubismExpressionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (Model && Model->Expression == this)
	{
		Model->Expression = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UCubismExpressionComponent::PostEditUndo()
{
	Super::PostEditUndo();

	const ACubismModel* Owner = Cast<ACubismModel>(GetOwner());

	Setup(Owner->Model);
}
#endif

void UCubismExpressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsControlledByUpdateController())
	{
		return;
	}

	if (!Model)
	{
		return;
	}

	Time += DeltaTime;

	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("Model is null."));
		return;
	}

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
				ParameterValue.OverwriteValue = Parameter->Value;

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
		float NewOverwriteValue = Model->GetParameter(ParameterValue.Id)->Value;

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

int32 UCubismExpressionComponent::GetExecutionOrder() const
{
	return CUBISM_EXECUTION_ORDER_EXPRESSION;
}

void UCubismExpressionComponent::OnCubismUpdate(float DeltaTime)
{
	if (!Model)
	{
		UE_LOG(LogTemp, Warning, TEXT("Model is null."));
		return;
	}

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
			for (int32 i = ExpressionQueue.Num() - 2; i >= 0; i--)
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
