#pragma once

#include "CoreMinimal.h"
#include "QuickAccessTool/Widgets/Sub/QuickItemWidget.h"
#include "Widgets/SCompoundWidget.h"

class SQuickItemWidget;
struct FSlateColorBrush;
DECLARE_DELEGATE_OneParam(FOnRefreshClick, const FString);

class QUICKACCESSTOOL_API SQuickPanelWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickPanelWidget)
			: _TabName(TEXT("Default"))
		{
		}

		SLATE_ARGUMENT(FString, TabName)
		SLATE_EVENT(FOnClick, AddNewTab)
		SLATE_EVENT(FOnClick, RemoveTab)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedPtr<SVerticalBox> QuickPanelVertical = nullptr;
	TSharedPtr<SImage> DragLine = nullptr;
	void AddChildren(const TArray<FString>& InPathArray, const int OffsetIndex = 0);
	void Refresh();
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
	void OnAddNewTabClicked() const;
	void OnRemoveTabClicked() const;
	void OnSaveClicked();
	void OnSaveAllClicked();
	void OnDeleteObject();
	void OnAddObjects(const TArray<FString>& NewPath, const int OffsetIndex = 0);
	void RenameTab(const FString& OldName,const FString& NewName);

	static TSharedRef<SWidget> GetObjThumbnailByPath(const FString& Path);
	TArray<FString>& GetPathArray() const;
	FString GetTabName() const { return TabName; }
private:
	FString TabName;

public:
	FOnClick AddNewTab;
	FOnClick RemoveTab;
	FOnRefreshClick OnRefreshClicked;

protected:
	TUniquePtr<FSlateColorBrush> SlateColorBrush = nullptr;
	TArray<TSharedPtr<SQuickItemWidget>> QuickItemWidgets = {};
	TArray<int32> QuickPanelSelectedIndexes = {};
	bool bIsDragging = false;
	bool bIsDragVisible = false;
	int32 FirstMenuSelectedIndex = -1;
	int32 LastFirstMenuSelectedIndex = -1;
};
