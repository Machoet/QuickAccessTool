#include "QuickAccessTool.h"
#include <functional>
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "QuickAccessToolStyle.h"
#include "QuickAccessToolCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "QuickAccessTool/Common/QuickAccessLibrary.h"
#include "QuickAccessTool/Widgets/QuickAccessWidget.h"
#include "QuickAccessTool/Language/Language.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

static const FName QuickAccessToolTabName("QuickAccessTool");

void FQuickAccessToolModule::StartupModule()
{
	FQuickAccessToolStyle::Initialize();
	FQuickAccessToolStyle::ReloadTextures();
	FQuickAccessToolCommands::Register();

	CreateCommandList();

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQuickAccessToolModule::RegisterMenus));
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(QuickAccessToolTabName,
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FQuickAccessToolModule::OnSpawnPluginTab)).
	                          SetDisplayName(QuickAccessToolLanguage::QuickAccessTool).SetMenuType(
		                          ETabSpawnerMenuType::Hidden);
}

void FQuickAccessToolModule::CreateCommandList()
{
	if (CommandList.IsValid())
	{
		return;
	}
	CommandList = MakeShareable(new FUICommandList);
	CommandList->MapAction(FQuickAccessToolCommands::Get().OpenQuickAccessTool,
	                       FExecuteAction::CreateStatic(&FQuickAccessToolModule::PluginButtonClicked),
	                       FCanExecuteAction());
	CommandList->MapAction(FQuickAccessToolCommands::Get().PastePicture,
	                       FExecuteAction::CreateStatic(&FQuickAccessToolModule::OnPastePictureClicked),
	                       FCanExecuteAction());
}

void FQuickAccessToolModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FQuickAccessToolStyle::Shutdown();
	FQuickAccessToolCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(QuickAccessToolTabName);
}

TSharedRef<SDockTab> FQuickAccessToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	SAssignNew(QuickAccessWidget, SQuickAccessWidget)
	.OnAddObjectClicked_Raw(this, &FQuickAccessToolModule::OnAddObjectClicked);
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			QuickAccessWidget.ToSharedRef()
		];
}

void FQuickAccessToolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(QuickAccessToolTabName);
}

void FQuickAccessToolModule::ShowExplorerClicked()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser");

	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);
	TArray<FString> SelectedPaths;
	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		FString RelativePath = FPackageName::LongPackageNameToFilename(SelectedAsset.PackagePath.ToString());
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(RelativePath);
		SelectedPaths.AddUnique(AbsolutePath);
	}

	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedFolders(SelectedFolders);
	for (const FString& SelectedFolder : SelectedFolders)
	{
		FString RelativePath = FPackageName::LongPackageNameToFilename(SelectedFolder);
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(RelativePath);
		SelectedPaths.AddUnique(AbsolutePath);
	}
	for (const FString& SelectedPath : SelectedPaths)
	{
		FPlatformProcess::ExploreFolder(*SelectedPath);
	}
}

void FQuickAccessToolModule::OnPastePictureClicked()
{
	UQuickAccessLibrary::SaveClipboardToAsset();
}

bool FQuickAccessToolModule::AddSelectedFiles() const
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);
	if (SelectedAssets.Num() <= 0)
	{
		return false;
	}
	TArray<FString> TempArray;
	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		TempArray.Add(SelectedAsset.ObjectPath.ToString());
	}

	if (QuickAccessWidget.IsValid())
	{
		QuickAccessWidget->OnAddObjects(TempArray);
	}
	return true;
}

TSharedRef<FExtender> FQuickAccessToolModule::OnExtendContentBrowserAssetSelectionMenu(
	const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	Extender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::Before,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FQuickAccessToolModule::AddQuickAccessMenuExtension)
	);

	return Extender;
}

void FQuickAccessToolModule::AddQuickAccessMenuExtension(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("QuickAccessTool", QuickAccessToolLanguage::QuickAccessMenuTitle);
	{
		MenuBuilder.AddMenuEntry(
			QuickAccessToolLanguage::AddToQuickAccessButton,
			QuickAccessToolLanguage::AddToQuickAccessTooltip,
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateRaw(this, &FQuickAccessToolModule::OnMenuAddObjectClicked),
				FCanExecuteAction()
			)
		);
	}
	MenuBuilder.EndSection();
}

FReply FQuickAccessToolModule::OnAddObjectClicked() const
{
	return AddSelectedFiles() ? FReply::Handled() : FReply::Unhandled();
}

void FQuickAccessToolModule::OnMenuAddObjectClicked() const
{
	AddSelectedFiles();
}

void FQuickAccessToolModule::RegisterMenus()
{
	{
#if ENGINE_MAJOR_VERSION == 4
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
#elif ENGINE_MAJOR_VERSION == 5
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
#endif
		{
			FToolMenuEntry& Entry = Section.AddEntry(
				FToolMenuEntry::InitToolBarButton(FQuickAccessToolCommands::Get().OpenQuickAccessTool));
			Entry.SetCommandList(CommandList);
		}
	}
	{
		const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(
			"LevelEditor");
		LevelEditorModule.GetGlobalLevelEditorActions()->Append(CommandList.ToSharedRef());
	}
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
			"ContentBrowser");
		ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(
			FContentBrowserMenuExtender_SelectedAssets::CreateRaw(
				this, &FQuickAccessToolModule::OnExtendContentBrowserAssetSelectionMenu));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQuickAccessToolModule, QuickAccessTool)
