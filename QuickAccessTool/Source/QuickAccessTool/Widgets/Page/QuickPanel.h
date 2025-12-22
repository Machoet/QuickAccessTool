// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FSlateColorBrush;
class SCustomItemObjectWidget;
/**
 * 
 */
class QUICKACCESSTOOL_API SQuickPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickPanel)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedPtr<SVerticalBox> QuickPanelVertical = nullptr;
	TSharedPtr<SImage> DragLine = nullptr;
	void AddChildren(const TArray<FString>& InPathArray, const bool bIsInit = false);

	void OnDragItem(const FVector2D Position, const float Offset, const int32 Index);
	void OnDragItemStart(const FVector2D Position, const float Offset, const int32 Index);
	void OnDragItemEnd(const FVector2D Position, const float Offset, const int32 Index);
	void OnItemClick(const int32 Index);

	void OnBrowseAssetClicked();
	bool OnAssetRemoved(const FAssetData& AssetData);
	void OnReferenceViewerClicked();
	void OnExploreFolderClicked();
	void OnClearAllFilesClicked();
	void OnSelectAllClicked();
	void OnSaveClicked();
	void OnSaveAllClicked();
	void OnDeleteObject();
	void OnAddObjects(const TArray<FString>& NewPath);

	static TSharedRef<SWidget> GetObjThumbnailByPath(const FString& Path);
protected:
	TUniquePtr<FSlateColorBrush> SlateColorBrush = nullptr;
	TArray<TSharedPtr<SCustomItemObjectWidget>> QuickAccessLineWidgets = {};
	TArray<int32> QuickPanelSelectedIndexes = {};
	bool bIsDragging = false;
	bool bIsDragVisible = false;
	int32 FirstMenuSelectedIndex = -1;
	int32 LastFirstMenuSelectedIndex = -1;
};
