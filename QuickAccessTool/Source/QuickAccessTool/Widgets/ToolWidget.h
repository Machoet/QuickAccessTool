// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SPaginatedWidget;
class SQuickCommandsPanel;
class SQuickCommonWidget;
class SQuickTaskWidget;
class SQuickPanelWidget;
class SMultiLineEditableText;
class SColorBlock;

class QUICKACCESSTOOL_API SToolWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SToolWidget)
		{
		}

		SLATE_EVENT(FOnClicked, OnAddObjectClicked)

	SLATE_END_ARGS()

	virtual ~SToolWidget() override;
	void Construct(const FArguments& InArgs);
	void OnAssetRemoved(const FAssetData& AssetData) const;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void OnRefresh(const FString& NewTabName);
	void OnAddNewTab();
	void BindEvent();
	void OnRemoveCurrentTab() const;
	void OnBrowseAssetClicked() const;
	void OnReferenceViewerClicked() const;
	void OnExploreFolderClicked() const;
	void OnClearAllFilesClicked();
	void OnSelectAllClicked() const;
	void OnSaveClicked() const;
	void OnSaveAllClicked() const;
	void OnDeleteObject();
	void EventOnKeyDown(const FKey& InKey) const;

	TSharedPtr<SWidget> CreateRightClickMenu();
	void CreateQuickPanelMenu(FMenuBuilder& MenuBuilder);
	void OnAddObjectsClick(TArray<FString> NewPath);
	int32 GetMenuWidgetIndex() const;
	FReply OnMenuClicked(const int32 Index);

private:
	TMap<FInputChord, TFunction<void()>> ChordFunctionMap = {};
	FOnClicked OnAddObjectClicked;
	TUniquePtr<FButtonStyle> TitleButtonStyle = nullptr;
	TUniquePtr<FTextBlockStyle> TitleBlockStyle = nullptr;
	TSharedPtr<SColorBlock> ColorPickerParentWidget;
	TUniquePtr<FTextBlockStyle>  CustomTextStyle;
	TSharedPtr<SHorizontalBox> MenuHorizontalBox = nullptr;
	TArray<TSharedPtr<SButton>> MenuButtons;

	TSharedPtr<SPaginatedWidget> QuickPanelPaginatedWidget;
	TSharedPtr<SQuickCommonWidget> QuickCommonWidget;
	TSharedPtr<SQuickTaskWidget> QuickTaskWidget;
	TSharedPtr<SQuickCommandsPanel> CustomCommandsPanel;
};
