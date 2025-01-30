/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModelActor.h"
#include "Model/CubismModelComponent.h"
#include "Model/CubismParameterComponent.h"
#include "Model/CubismPartComponent.h"

class SCubismModelParameterControllerWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCubismModelParameterControllerWindow)
		: _ModelActor(nullptr)
	{}
		SLATE_ARGUMENT(TWeakObjectPtr<ACubismModel>, ModelActor)
	SLATE_END_ARGS()

	~SCubismModelParameterControllerWindow();

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** Called when actor selection changes in editor. */
	void HandleEditorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh);

	/** Refresh the UI layout after changing the current model actor or its parameters. */
	void RefreshTabs();

	/** Build the parameter list UI. */
	TSharedPtr<SWidget> BuildParameterList();

	/** Build the part list UI. */
	TSharedPtr<SWidget> BuildPartList();

	/** Callback when a parameter numeric value changes by user input. */
	void OnParameterNumericValueCommitted(float NewValue, const FString& ParameterId);

	/** Callback when a parameter slider value changes by user input. */
	void OnParameterSliderValueChanged(float NewValue, const FString& ParameterId);

	/** Callback when a part numeric value changes by user input. */
	void OnPartNumericValueCommitted(float NewValue, const FString& PartId);

	/** Callback when a part slider value changes by user input. */
	void OnPartSliderValueChanged(float NewValue, const FString& PartId);

	/** Switch tabs (0 = Parameters, 1 = Parts). */
	void SetCurrentTab(int32 NewTabIndex);

private:
	/** Currently selected model actor. */
	TWeakObjectPtr<ACubismModel> ModelActor;

	/** The widget switcher for tabs */
	TSharedPtr<SWidgetSwitcher> TabContentSwitcher;

	/** SBox for Parameters tab content */
	TSharedPtr<SBox> ParametersContainer;

	/** SBox for Parts tab content */
	TSharedPtr<SBox> PartsContainer;

	/** Current selected tab (0: Parameters, 1: Parts) */
	int32 CurrentTab = 0;
};
