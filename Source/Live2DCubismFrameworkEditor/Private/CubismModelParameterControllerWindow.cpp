/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismModelParameterControllerWindow.h"

#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "Model/CubismParameterStoreComponent.h"
#include "LevelEditor.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SNumericEntryBox.h"

void SCubismModelParameterControllerWindow::Construct(const FArguments& InArgs)
{
	ModelActor = InArgs._ModelActor;

	SAssignNew(ParametersContainer, SBox);
	SAssignNew(PartsContainer, SBox);

	ChildSlot
	[
		SNew(SVerticalBox)

		// Model Label
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (ModelActor.IsValid())
				{
					return FText::FromString(ModelActor->GetActorLabel());
				}
				return FText();
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 5.0f)
		[
			SNew(SSeparator)
		]

		// Tabs (Parameters / Parts)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f, 5.0f)
		[
			SNew(SHorizontalBox)

			// Parameter Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Parameters"))
				.OnClicked_Lambda([this]()
				{
					SetCurrentTab(0);
					return FReply::Handled();
				})
				.ButtonColorAndOpacity_Lambda([this]()
				{
					return CurrentTab == 0 ? FLinearColor::White : FLinearColor::Gray;
				})
			]

			// Parts Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f,0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Parts"))
				.OnClicked_Lambda([this]()
				{
					SetCurrentTab(1);
					return FReply::Handled();
				})
				.ButtonColorAndOpacity_Lambda([this]()
				{
					return CurrentTab == 1 ? FLinearColor::White : FLinearColor::Gray;
				})
			]
		]

		// Tab Content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TabContentSwitcher, SWidgetSwitcher)
			.WidgetIndex(CurrentTab)

			// Parameters Tab
			+ SWidgetSwitcher::Slot()
			[
				ParametersContainer.ToSharedRef()
			]

			// Parts Tab
			+ SWidgetSwitcher::Slot()
			[
				PartsContainer.ToSharedRef()
			]
		]
	];

	// Register the actor selection change delegate from the LevelEditorModule
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &SCubismModelParameterControllerWindow::HandleEditorSelectionChanged);
	}

	RefreshTabs();
}

void SCubismModelParameterControllerWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

SCubismModelParameterControllerWindow::~SCubismModelParameterControllerWindow()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.OnActorSelectionChanged().RemoveAll(this);
	}
}

void SCubismModelParameterControllerWindow::HandleEditorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
	for (UObject* Obj : NewSelection)
	{
		AActor* Actor = Cast<AActor>(Obj);
		if (ACubismModel* CubismActor = Cast<ACubismModel>(Actor))
		{
			ModelActor = CubismActor;
			break;
		}
	}

	RefreshTabs();
}

void SCubismModelParameterControllerWindow::RefreshTabs()
{
	if (!TabContentSwitcher.IsValid() || !ParametersContainer.IsValid() || !PartsContainer.IsValid())
	{
		return;
	}

	if (!ModelActor.IsValid())
	{
		ParametersContainer->SetContent(
			SNew(STextBlock).Text(FText::FromString("No Model Actor specified."))
		);
		PartsContainer->SetContent(
			SNew(STextBlock).Text(FText::FromString("No Model Actor specified."))
		);
		return;
	}

	UCubismModelComponent* Model = ModelActor->Model;
	if (!Model)
	{
		ParametersContainer->SetContent(
			SNew(STextBlock).Text(FText::FromString("UCubismModelComponent not found on the target actor."))
		);
		PartsContainer->SetContent(
			SNew(STextBlock).Text(FText::FromString("UCubismModelComponent not found on the target actor."))
		);
		return;
	}

	ParametersContainer->SetContent(
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(10.0f)
		[
			BuildParameterList().ToSharedRef()
		]
	);

	PartsContainer->SetContent(
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(10.0f)
		[
			BuildPartList().ToSharedRef()
		]
	);
}

TSharedPtr<SWidget> SCubismModelParameterControllerWindow::BuildParameterList()
{
	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	if (!ModelActor.IsValid() || !ModelActor->Model)
	{
		return SNullWidget::NullWidget;
	}

	UCubismModelComponent* Model = ModelActor->Model;

	for (UCubismParameterComponent* Parameter : Model->Parameters)
	{
		VerticalBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.MaxWidth(300.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([Model, Parameter]()
				{
					if (Model->DisplayInfoJson)
					{
						for (const FCubismDisplayInfoParameter& DisplayInfoParameter : Model->DisplayInfoJson->Parameters)
						{
							if (DisplayInfoParameter.Id == Parameter->Id)
							{
								return DisplayInfoParameter.Name;
							}
						}
					}

					return FText::FromString(Parameter->Id);
				})
				.Justification(ETextJustify::Left)
				.MinDesiredWidth(150.0f)
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(100.0f)
			.Padding(5.0f, 0.0f)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([Parameter]() { return FCString::Atof(*FString::Printf(TEXT("%.4g"), Parameter->Value)); })
				.OnValueCommitted_Lambda([this, Parameter](float NewValue, ETextCommit::Type CommitType)
				{
					if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
					{
						OnParameterNumericValueCommitted(NewValue, Parameter->Id);
					}
				})
				.MinValue(Parameter->MinimumValue)
				.MaxValue(Parameter->MaximumValue)
				.MinSliderValue(Parameter->MinimumValue)
				.MaxSliderValue(Parameter->MaximumValue)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2f"), Parameter->MinimumValue)))
				.MinDesiredWidth(50.0f)
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(300.0f)
			.Padding(2.0f, 0.0f)
			[
				SNew(SSlider)
				.MinValue(Parameter->MinimumValue)
				.MaxValue(Parameter->MaximumValue)
				.Value_Lambda([Parameter]() { return Parameter->Value; })
				.OnValueChanged_Lambda([this, Parameter](float NewValue) { OnParameterSliderValueChanged(NewValue, Parameter->Id); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2f"), Parameter->MaximumValue)))
				.MinDesiredWidth(50.0f)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Reset"))
				.OnClicked_Lambda([this, Parameter]()
				{
					if (ModelActor.IsValid() && ModelActor->Model && ModelActor->Model->ParameterStore)
					{
						UCubismModelComponent* Model = ModelActor->Model;
						int32 Index = Model->GetParameterIndex(Parameter->Id);

						Parameter->SetParameterValue(Parameter->DefaultValue);
						Model->ParameterStore->SaveParameterValue(Index);
					}
					return FReply::Handled();
				})
			]
		];
	}

	return VerticalBox;
}

TSharedPtr<SWidget> SCubismModelParameterControllerWindow::BuildPartList()
{
	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	if (!ModelActor.IsValid() || !ModelActor->Model)
	{
		return SNullWidget::NullWidget;
	}

	UCubismModelComponent* Model = ModelActor->Model;

	for (UCubismPartComponent* Part : Model->Parts)
	{
		VerticalBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)

			// Part Id
			+ SHorizontalBox::Slot()
			.MaxWidth(300.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([Model, Part]()
				{
					if (Model->DisplayInfoJson)
					{
						for (const FCubismDisplayInfoPart& DisplayInfoPart : Model->DisplayInfoJson->Parts)
						{
							if (DisplayInfoPart.Id == Part->Id)
							{
								return DisplayInfoPart.Name;
							}
						}
					}

					return FText::FromString(Part->Id);
				})
				.Justification(ETextJustify::Left)
				.MinDesiredWidth(150.0f)
			]

			// Numeric Entry Box for Opacity
			+ SHorizontalBox::Slot()
			.MaxWidth(100.0f)
			.Padding(5.0f, 0.0f)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([Part]() { return FCString::Atof(*FString::Printf(TEXT("%.4g"), Part->Opacity)); })
				.OnValueCommitted_Lambda([this, Part](float NewValue, ETextCommit::Type CommitType)
				{
					if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
					{
						OnPartNumericValueCommitted(NewValue, Part->Id);
					}
				})
				.MinValue(0.0f)
				.MaxValue(1.0f)
				.MinSliderValue(0.0f)
				.MaxSliderValue(1.0f)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f,0.0f,0.0f,0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("0.0"))
				.MinDesiredWidth(50.0f)
			]

			// Opacity Slider
			+ SHorizontalBox::Slot()
			.MaxWidth(300.0f)
			.Padding(2.0f, 0.0f)
			[
				SNew(SSlider)
				.MinValue(0.0f)
				.MaxValue(1.0f)
				.Value_Lambda([Part]() { return Part->Opacity; })
				.OnValueChanged_Lambda([this, Part](float NewValue) { OnPartSliderValueChanged(NewValue, Part->Id); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f,0.0f,0.0f,0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("1.0"))
				.MinDesiredWidth(50.0f)
			]
		];
	}

	return VerticalBox;
}

void SCubismModelParameterControllerWindow::OnParameterNumericValueCommitted(float NewValue, const FString& ParameterId)
{
	if (ModelActor.IsValid() && ModelActor->Model && ModelActor->Model->ParameterStore)
	{
		UCubismModelComponent* Model = ModelActor->Model;

		UCubismParameterComponent* Parameter = Model->GetParameter(ParameterId);
		int32 Index = Model->GetParameterIndex(ParameterId);

		if (Parameter)
		{
			NewValue = FMath::Clamp(NewValue, Parameter->MinimumValue, Parameter->MaximumValue);
			Parameter->SetParameterValue(NewValue);
			Model->ParameterStore->SaveParameterValue(Index);
		}
	}
}

void SCubismModelParameterControllerWindow::OnParameterSliderValueChanged(float NewValue, const FString& ParameterId)
{
	if (ModelActor.IsValid() && ModelActor->Model && ModelActor->Model->ParameterStore)
	{
		UCubismModelComponent* Model = ModelActor->Model;

		UCubismParameterComponent* Parameter = Model->GetParameter(ParameterId);

		if (Parameter)
		{
			Parameter->SetParameterValue(NewValue);
			Model->ParameterStore->SaveParameterValue(Model->GetParameterIndex(ParameterId));
		}
	}
}

void SCubismModelParameterControllerWindow::OnPartNumericValueCommitted(float NewValue, const FString& PartId)
{
	if (ModelActor.IsValid() && ModelActor->Model && ModelActor->Model->ParameterStore)
	{
		UCubismModelComponent* Model = ModelActor->Model;

		UCubismPartComponent* Part = Model->GetPart(PartId);

		if (Part)
		{
			NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
			Part->SetPartOpacity(NewValue);
			Model->ParameterStore->SavePartOpacity(Model->GetPartIndex(PartId));
		}
	}
}

void SCubismModelParameterControllerWindow::OnPartSliderValueChanged(float NewValue, const FString& PartId)
{
	if (ModelActor.IsValid() && ModelActor->Model && ModelActor->Model->ParameterStore)
	{
		UCubismModelComponent* Model = ModelActor->Model;

		UCubismPartComponent* Part = Model->GetPart(PartId);

		if (Part)
		{
			Part->SetPartOpacity(NewValue);
			Model->ParameterStore->SavePartOpacity(Model->GetPartIndex(PartId));
		}
	}
}

void SCubismModelParameterControllerWindow::SetCurrentTab(int32 NewTabIndex)
{
	CurrentTab = NewTabIndex;
	if (TabContentSwitcher.IsValid())
	{
		TabContentSwitcher->SetActiveWidgetIndex(CurrentTab);
	}
}
