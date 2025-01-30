/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismModelParameterControllerCommands.h"

#include "CubismModelParameterControllerWindow.h"
#include "Model/CubismModelActor.h"
#include "Selection.h"

#include "CubismLog.h"

#define LOCTEXT_NAMESPACE "Live2DCubismFrameworkEditorModule"

DEFINE_LOG_CATEGORY(LogCubism)

static const FName CubismModelParameterControllerTabName("CubismModelParameterControllerTab");

class FLive2DCubismFrameworkEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FCubismModelParameterControllerCommands::Register();

		CommandList = MakeShared<FUICommandList>();

		CommandList->MapAction(
			FCubismModelParameterControllerCommands::Get().OpenPluginWindow,
			FExecuteAction::CreateRaw(this, &FLive2DCubismFrameworkEditorModule::InvokeCubismModelParameterController),
			FCanExecuteAction()
		);

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLive2DCubismFrameworkEditorModule::RegisterMenus));

		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CubismModelParameterControllerTabName, FOnSpawnTab::CreateRaw(this, &FLive2DCubismFrameworkEditorModule::OnSpawnPluginTab))
			.SetDisplayName(LOCTEXT("FLive2DCubismFrameworkEditorTabTitle", "Cubism Model Parameter Controller"))
			.SetTooltipText(LOCTEXT("Live2DCubismFrameworkEditorTabTooltipText", "Open the Cubism model parameter controller tab."))
			.SetMenuType(ETabSpawnerMenuType::Hidden)
			.SetIcon(FSlateIcon());
	}

	virtual void ShutdownModule() override
	{
		FCubismModelParameterControllerCommands::Unregister();

		UToolMenus::UnregisterOwner(this);

		UToolMenus::UnRegisterStartupCallback(this);

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CubismModelParameterControllerTabName);
	}

private:
	void InvokeCubismModelParameterController()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(CubismModelParameterControllerTabName);
	}

	void RegisterMenus()
	{
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
			{
				FToolMenuSection& Section = Menu->AddSection("Live2DCubismModel", LOCTEXT("Live2DCubismModelSection", "Live2D Cubism SDK"));
				Section.AddMenuEntryWithCommandList(FCubismModelParameterControllerCommands::Get().OpenPluginWindow, CommandList);   
			}
		}

		{
			UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
			{
				FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Live2DCubismModel");
				{
					FToolMenuEntry& Entry = Section.AddEntry(
						FToolMenuEntry::InitToolBarButton(FCubismModelParameterControllerCommands::Get().OpenPluginWindow)
					);
					Entry.SetCommandList(CommandList);
				}
			}
		}

		{
			UToolMenu* ActorMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
			
			FToolMenuSection& Section = ActorMenu->AddSection("Live2DCubismModel", LOCTEXT("Live2DCubismModelSection", "Live2D Cubism SDK"));

			FUIAction Action(
				FExecuteAction::CreateRaw(this, &FLive2DCubismFrameworkEditorModule::InvokeCubismModelParameterController),
				FCanExecuteAction::CreateLambda([]()
				{
					if (GEditor && GEditor->GetSelectedActors()->Num() > 0)
					{
						// check if the selected actor is a Cubism model actor
						return Cast<ACubismModel>(GEditor->GetSelectedActors()->GetTop(ACubismModel::StaticClass())) != nullptr;
					}
					return false;
				})
			);

			Section.AddMenuEntry(
				"OpenCubismModelParameterControllerFromContext",
				LOCTEXT("OpenCubismModelParameterControllerFromContext", "Open Cubism Model Parameter Controller"),
				LOCTEXT("OpenCubismModelParameterControllerFromContextTooltip", "Open the Cubism model parameter controller for the selected model actor."),
				FSlateIcon(),
				Action
			);
		}
	}

	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		TWeakObjectPtr<ACubismModel> SelectedActor = nullptr;

		if (GEditor && GEditor->GetSelectedActors()->Num() > 0)
		{
			if (ACubismModel* Selection = Cast<ACubismModel>(GEditor->GetSelectedActors()->GetTop(ACubismModel::StaticClass())))
			{
				SelectedActor = Selection;
			}
		}
		
		TSharedRef<SDockTab> ResultTab = SNew(SDockTab)
			.Label(LOCTEXT("CubismModelParameterControllerTabTitle", "Cubism Model Parameter Controller"))
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SCubismModelParameterControllerWindow)
				.ModelActor(SelectedActor)
			];
		
		return ResultTab;
	}

private:
	TSharedPtr<class FUICommandList> CommandList;
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLive2DCubismFrameworkEditorModule, Live2DCubismFrameworkEditor)
