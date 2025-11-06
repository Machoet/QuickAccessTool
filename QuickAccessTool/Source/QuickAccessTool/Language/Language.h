#pragma once

#define LOCTEXT_NAMESPACE "QuickAccessTool"

namespace QuickAccessToolLanguage
{
	static const FText QuickPanel = LOCTEXT("QuickPanel", "Quick Panel");
	static const FText QuickAccessTool = LOCTEXT("QuickAccessTool", "Quick Access Tool");
	
	static const FText LoadAssert = LOCTEXT("LoadAssert", "Loading Assets...");
	static const FText Loading = LOCTEXT("Loading", "Loading {0}...");
	
	static const FText QuickAccessMenuTitle = LOCTEXT("QuickAccessSection", "Quick Access");

	static const FText AddToQuickAccessButton = LOCTEXT("AddToQuickAccess", "Add to Quick Access Tool");
	static const FText AddToQuickAccessTooltip = LOCTEXT("AddToQuickAccessTooltip", "Add the selected assets to the quick access list.");

	static const FText NoAssetsSelected = LOCTEXT("NoAssetsSelected", "No assets selected. Please select some assets to add.");
	static const FText QuickAccessUpdated = LOCTEXT("QuickAccessUpdated", "Quick access list updated successfully.");

	static const FText SelectAllFilesFormat = LOCTEXT("SelectAllFilesFormat", "Select All Files{0}");
	static const FText SelectAllFilesTooltip = LOCTEXT("SelectAllFilesTooltip", "Select All Files In Tool");
	
	static const FText ClearAllFilesFormat = LOCTEXT("ClearAllFilesFormat", "Clear All Files{0}");
	static const FText ClearAllFilesTooltip = LOCTEXT("ClearAllFilesTooltip", "Clear All Files In Tool");

	static const FText BrowseToAssetFormat = LOCTEXT("BrowseToAssetFormat", "Browse To Asset{0}");
	static const FText BrowseToTooltipFormat = LOCTEXT("BrowseToTooltipFormat", "Browse To: {0}");

	static const FText OpenFileFormat = LOCTEXT("OpenFileFormat", "Open File{0}");
	static const FText OpenFileTooltipFormat = LOCTEXT("OpenFileTooltipFormat", "Open:{0}");

	static const FText ReferenceViewFormat = LOCTEXT("ReferenceViewFormat", "Reference View...{0}");
	static const FText ReferenceViewTooltipFormat = LOCTEXT("ReferenceViewTooltipFormat", "Reference View: {0}");

	static const FText CopyFileName = LOCTEXT("CopyFileName", "Copy File Name");
	static const FText CopyFileNameTooltipFormat = LOCTEXT("CopyFileNameTooltipFormat", "Copy File Name: {0}");

	static const FText CopyFileReference = LOCTEXT("CopyFileReference", "Copy File Reference");
	static const FText CopyFileReferenceTooltipFormat = LOCTEXT("CopyFileReferenceTooltipFormat", "Copy File Reference: {0}");

	static const FText CopyFilePath = LOCTEXT("CopyFilePath", "Copy File Path");
	static const FText CopyFilePathTooltipFormat = LOCTEXT("CopyFilePathTooltipFormat", "Copy File Path: {0}");

	static const FText ShowInExplorerFormat = LOCTEXT("ShowInExplorerFormat", "Show In Explorer{0}");
	static const FText ShowInExplorerTooltipFormat = LOCTEXT("ShowInExplorerTooltipFormat", "Show In Explorer: {0}");
	
	static const FText Common = LOCTEXT("Common", "Common");
	static const FText Language = LOCTEXT("Language", "Language");
	static const FText LessCPU = LOCTEXT("LessCPU", "Use Less CPU when in Background");
	
	static const FText OpenColorPicker = LOCTEXT("OpenColorPicker", "Open Color Picker");
	static const FText CopyColorToClipboard = LOCTEXT("CopyColorToClipboard", "Copy Color To Clipboard");
	
	static const FText SaveFile = LOCTEXT("SaveFile", "Save Select Files{0}");
	static const FText SaveFileToolTips = LOCTEXT("SaveFileToolTips", "Save Select Files");
	
	static const FText SaveAllFiles = LOCTEXT("SaveAllFiles", "Save All Files{0}");
	static const FText SaveAllFilesTipsToolTips = LOCTEXT("SaveAllFilesTipsToolTips", "Save All Files");
	
	static const FText CustomTask = LOCTEXT("CustomTask", "Custom Task");
	static const FText AddNewTask = LOCTEXT("AddNewTask", "Please add a new task...");
	
	static const FText ClipboardTexture = LOCTEXT("ClipboardTexture", "Clipboard Texture{0}");
	static const FText ClipboardSaveAsTextureToolTips = LOCTEXT("ClipboardSaveAsTextureToolTips", "The clipboard content will be saved as a Texture.");
	static const FText ClipboardNotHasPictures = LOCTEXT("ClipboardNotHasPictures", "There are no pictures on the clipboard.");
}

#undef LOCTEXT_NAMESPACE
