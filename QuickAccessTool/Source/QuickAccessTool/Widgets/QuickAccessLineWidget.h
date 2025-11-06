// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnClick);
DECLARE_DELEGATE_OneParam(FOnItemClick, const int32);
DECLARE_DELEGATE_ThreeParams(FOnItemDrag, const FVector2D, const float, const int32);

class SQuickAccessButton;

class QUICKACCESSTOOL_API SQuickAccessLineWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickAccessLineWidget)
			: _Path(""),
			  _Text(FText::GetEmpty()),
			  _IsSelected(false),
			  _IconWidget(nullptr),
			  _Index(-1)
		{
		}

		SLATE_ATTRIBUTE(FString, Path)
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_ATTRIBUTE(bool, IsSelected)
		SLATE_ATTRIBUTE(TSharedPtr<SWidget>, IconWidget)
		SLATE_ATTRIBUTE(int32, Index)
		SLATE_EVENT(FOnItemClick, OnClicked)
		SLATE_EVENT(FOnItemDrag, OnItemDrag)
		SLATE_EVENT(FOnItemDrag, OnItemDragStart)
		SLATE_EVENT(FOnItemDrag, OnItemDragEnd)
		SLATE_EVENT(FOnClick, OnSelectAllClicked)
		SLATE_EVENT(FOnClick, OnClearAllClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double CurrentTime, float DeltaTime) override;
	void OnButtonPressed();
	void OnButtonReleased();
	FReply OnButtonClick() const;
	FReply OnButtonDoubleClick() const;
	void SetSelected(const bool bIsSelected);
	void RefreshButtonState();
	bool GetIsSelected() const;
	void BrowserToObject() const;
	void ReferenceViewer() const;
	void ExploreFolder() const;
	void Save() const;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	TSharedPtr<SWidget> CreateRightClickMenu();
	void SetIndex(const int32 InIndex);
	int32 GetIndex() const;
	UObject* GetObject() const;
	static float GetIconSize() { return GetSize() - GetOffset() * 2; }
	static float GetSize() { return 30.0f; }
	static float GetOffset() { return 2.0f; }
	const FSlateBrush* GetDirtyImage() const;

private:
	TSharedPtr<SQuickAccessButton> Button;
	TSharedPtr<STextBlock> TextBlock;
	FButtonStyle WidgetStyle;

	FSlateColor NormalTintColor = FLinearColor(0, 0, 0, 0);
	FSlateColor HoveredTintColor = FLinearColor(1, 1, 1, 0.2);
	FSlateColor PressedTintColor = FLinearColor(1, 0.4, 0, 0.65);

	FText Text = FText::GetEmpty();
	bool IsSelected = false;

	FOnItemClick OnClicked;
	FOnItemDrag OnItemDrag;
	FOnItemDrag OnItemDragStart;
	FOnItemDrag OnItemDragEnd;
	FOnClick OnSelectAllClicked;
	FOnClick OnClearAllClicked;

	TUniquePtr<FSlateBrush> AssetDirtyBrush = nullptr;
	FVector2D DragPosition = FVector2D(0, 0);
	FString Path = FString("");
	int32 Index = -1;
	TSharedPtr<SWidget> IconWidget;
	bool bIsDragging = false;
	FVector2D InitialMousePosition = FVector2D(0, 0);
	FVector2D DragOffset = FVector2D(0, 0);
};
