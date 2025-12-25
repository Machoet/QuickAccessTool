// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "QuickAccessTool/Archive/QuickAccessArchive.h"

class SToolWidget;

class FQuickAccessToolModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	void CreateCommandList();

	virtual void ShutdownModule() override;

	static void PluginButtonClicked();

	static void ShowExplorerClicked();

	static void OnPastePictureClicked();

	bool AddSelectedFiles() const;

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	void AddQuickAccessMenuExtension(FMenuBuilder& MenuBuilder);

	FReply OnAddObjectClicked() const;

	void OnMenuAddObjectClicked() const;

	void EventOnKeyDown(const FKey& InKey) const;

	static FQuickAccessArchiveInfo QuickAccessArchiveInfo;

private:
	void RegisterMenus();

	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<FUICommandList> CommandList = nullptr;

	TSharedPtr<SToolWidget> ToolWidget = nullptr;

	TSharedPtr<class FQuickAccessInputProcessor> InputProcessor;
};
