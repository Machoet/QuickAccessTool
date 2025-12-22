// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SCompoundWidget.h"

class SCustomCommandsPanel;
class SQuickCommonWidget;
class SQuickTaskWidget;
class SQuickPanel;
class SMultiLineEditableText;
class SCustomItemObjectWidget;
class SColorBlock;

class QUICKACCESSTOOL_API SToolWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SToolWidget)
		{
		}

		SLATE_EVENT(FOnClicked, OnAddObjectClicked)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void OnAssetRemoved(const FAssetData& AssetData) const;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void OnBrowseAssetClicked() const;
	void OnReferenceViewerClicked() const;
	void OnExploreFolderClicked() const;
	void OnClearAllFilesClicked();
	void OnSelectAllClicked() const;
	void OnSaveClicked() const;
	void OnSaveAllClicked() const;
	void OnDeleteObject();

	TSharedPtr<SWidget> CreateRightClickMenu();
	void CreateQuickPanelMenu(FMenuBuilder& MenuBuilder);
	void OnAddObjects(TArray<FString> NewPath);
	int32 GetMenuWidgetIndex() const;
	FReply OnMenuClicked(const int32 Index);

private:
	TMap<FInputChord, TFunction<void()>> ChordFunctionMap = {};
	FOnClicked OnAddObjectClicked;
	TUniquePtr<FButtonStyle> TitleButtonStyle = nullptr;
	TUniquePtr<FTextBlockStyle> TitleBlockStyle = nullptr;
	TSharedPtr<SColorBlock> ColorPickerParentWidget;

	TSharedPtr<SHorizontalBox> MenuHorizontalBox = nullptr;
	TArray<TSharedPtr<SButton>> MenuButtons;

	TSharedPtr<SQuickPanel> QuickPanel;
	TSharedPtr<SQuickCommonWidget> QuickCommonWidget;
	TSharedPtr<SQuickTaskWidget> QuickTaskWidget;
	TSharedPtr<SCustomCommandsPanel> CustomCommandsPanel;
};
