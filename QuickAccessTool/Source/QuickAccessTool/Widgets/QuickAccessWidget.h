// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "QuickAccessTool/Archive/QuickAccessArchive.h"
#include "QuickAccessTool/Language/Language.h"
#include "Widgets/SCompoundWidget.h"

class SMultiLineEditableText;
class SQuickAccessLineWidget;
class SColorBlock;

class QUICKACCESSTOOL_API SQuickAccessWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickAccessWidget)
		{
		}

		SLATE_EVENT(FOnClicked, OnAddObjectClicked)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void OnTaskTextChanged(const FText& NewText);
	void OnAssetRemoved(const FAssetData& AssetData);

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	void OnItemClick(const int32 Index);
	void OnDragItem(const FVector2D Position, const float Offset, const int32 Index);
	void OnDragItemStart(const FVector2D Position, const float Offset, const int32 Index);
	void OnDragItemEnd(const FVector2D Position, const float Offset, const int32 Index);
	static TSharedRef<SWidget> GetObjThumbnailByPath(const FString& Path);
	void OnBrowseAssetClicked();
	void OnReferenceViewerClicked();
	void OnExploreFolderClicked();
	void OnClearAllFilesClicked();
	void OnSelectAllClicked();
	void OnSaveClicked();
	void OnSaveAllClicked();

	TSharedPtr<SWidget> CreateRightClickMenu();
	void CreateQuickPanelMenu(FMenuBuilder& MenuBuilder);
	void OnDeleteObject();
	void OnAddObjects(TArray<FString> NewPath);
	const TArray<FString>& GetPathArray();
	void AddChildren(TArray<FString> InPathArray, const bool bIsInit = false);
	int32 GetMenuWidgetIndex() const;
	FReply OnMenuClicked(const int32 Index);
	FReply OnPickColorClicked();
	void OnSelectedStopColorChanged(const FLinearColor InNewColor);
	void OnCancelSelectedStopColorChange(FLinearColor PreviousColor);

private:
	TSharedPtr<SVerticalBox> QuickAccessLineVertical = nullptr;
	TArray<TSharedPtr<SQuickAccessLineWidget>> QuickAccessLineWidgets = {};
	int32 FirstMenuSelectedIndex = -1;
	int32 LastFirstMenuSelectedIndex = -1;
	TMap<FInputChord, TFunction<void()>> ChordFunctionMap = {};
	TArray<int32> FirstMenuSelectedIndexes = {};
	TSharedPtr<SImage> DragLine = nullptr;
	FOnClicked OnAddObjectClicked;
	bool bIsDragVisible = false;
	bool bIsDragging = false;
	TUniquePtr<FSlateColorBrush> SlateColorBrush = nullptr;
	TUniquePtr<FButtonStyle> TitleButtonStyle = nullptr;
	TUniquePtr<FTextBlockStyle> TitleBlockStyle = nullptr;
	TUniquePtr<FButtonStyle> ItemButtonStyle = nullptr;
	TArray<TSharedPtr<FString>> LanguageOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> LanguageComboBox;
	TSharedPtr<SColorBlock> ColorPickerParentWidget;
	FLinearColor StartColor = FLinearColor::White;
	FLinearColor SelectColor = FLinearColor::White;

	TSharedPtr<SHorizontalBox> MenuHorizontalBox = nullptr;
	TArray<TSharedPtr<SButton>> MenuButtons;
	TArray<FText> MenuTexts = {QuickAccessToolLanguage::QuickPanel, QuickAccessToolLanguage::Common, QuickAccessToolLanguage::CustomTask};
	TSharedPtr<SMultiLineEditableText> CustomTaskMultiLineEditableText = nullptr;

	FQuickAccessArchiveInfo QuickAccessArchiveInfo;
};
