#include "QuickPanelWidget.h"
#include "SlateOptMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Brushes/SlateColorBrush.h"
#include "Misc/ScopedSlowTask.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "QuickAccessTool/Widgets/Sub/QuickItemWidget.h"
#include "AssetThumbnail.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickPanelWidget::Construct(const FArguments& InArgs)
{
	TabName = InArgs._TabName;
	SAssignNew(QuickPanelVertical, SVerticalBox);

	FirstMenuSelectedIndex = -1;
	QuickItemWidgets.Empty();
	AddNewTab = InArgs._AddNewTab;
	RemoveTab = InArgs._RemoveTab;
	AddChildren(GetPathArray());

	SlateColorBrush = MakeUnique<FSlateColorBrush>(FLinearColor::Yellow);
	SlateColorBrush->SetImageSize(FVector2D(1, 1));

	SAssignNew(DragLine, SImage)
	.Visibility(EVisibility::Collapsed)
	.Image(SlateColorBrush.Get());

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		[
			QuickPanelVertical.ToSharedRef()
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		.Padding(0.0f, SQuickItemWidget::GetSize(), 0.f, 0.f)
		[
			DragLine.ToSharedRef()
		]
	];
}

TArray<FString>& SQuickPanelWidget::GetPathArray() const
{
	auto& MultiMap = FQuickAccessToolModule::QuickAccessArchiveInfo.MultiPathMap;
	if (!MultiMap.Contains(TabName))
	{
		MultiMap.Add(TabName, TArray<FString>());
	}
	return MultiMap[TabName];
}

void SQuickPanelWidget::AddChildren(const TArray<FString>& InPathArray, const int OffsetIndex)
{
	if (InPathArray.Num() <= 0)
	{
		return;
	}
	FScopedSlowTask SlowTask(InPathArray.Num(), QuickAccessToolLanguage::LoadAssert, true);
	SlowTask.MakeDialog();

	TWeakPtr<SQuickPanelWidget> TempThis = SharedThis(this);
	for (int32 Index = 0; Index < InPathArray.Num(); Index++)
	{
		const FString& Path = InPathArray[Index];

		SlowTask.EnterProgressFrame(1, FText::Format(QuickAccessToolLanguage::Loading, FText::FromString(Path)));

		if (SlowTask.ShouldCancel())
		{
			break;
		}

		const UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), nullptr, *Path);
		TSharedPtr<SQuickItemWidget> TempQuickItemWidget;
		SAssignNew(TempQuickItemWidget, SQuickItemWidget)
		.TabName(TabName)
		.Path(Path)
		.Index(Index + OffsetIndex)
		.Text(FText::FromString(LoadedObj == nullptr ? "None" : LoadedObj->GetName()))
		.IconWidget(GetObjThumbnailByPath(Path))
		.OnItemDrag(this, &SQuickPanelWidget::OnDragItem)
		.OnItemDragStart(this, &SQuickPanelWidget::OnDragItemStart)
		.OnItemDragEnd(this, &SQuickPanelWidget::OnDragItemEnd)
		.OnClicked(this, &SQuickPanelWidget::OnItemClick)
		.OnClearAllClicked(this, &SQuickPanelWidget::OnClearAllFilesClicked)
		.OnSelectAllClicked(this, &SQuickPanelWidget::OnSelectAllClicked)
		.OnAddNewTabClicked(this, &SQuickPanelWidget::OnAddNewTabClicked)
		.OnRemoveTabClicked(this, &SQuickPanelWidget::OnRemoveTabClicked)
		.OnMoveToClick_Lambda([TempThis](const FString& NewTabName, const int32 Index)
		{
			if (!TempThis.IsValid()) return;
			const TSharedPtr<SQuickPanelWidget> Panel = TempThis.Pin();

			TArray<FString>& CurrentPathArray = Panel->GetPathArray();
			auto& AllDataMap = FQuickAccessToolModule::QuickAccessArchiveInfo.MultiPathMap;

			TArray<FString>& TargetPathArray = AllDataMap.FindOrAdd(NewTabName);

			Panel->QuickPanelSelectedIndexes.AddUnique(Index);
			Panel->QuickPanelSelectedIndexes.Sort([](const int32& A, const int32& B) { return A > B; });

			TArray<FString> PathsToMove;
			for (const int32 SelectIndex : Panel->QuickPanelSelectedIndexes)
			{
				if (Panel->QuickItemWidgets.IsValidIndex(SelectIndex) && CurrentPathArray.IsValidIndex(SelectIndex))
				{
					FString PathToMove = CurrentPathArray[SelectIndex];
					PathsToMove.Add(PathToMove);
					Panel->QuickPanelVertical->RemoveSlot(Panel->QuickItemWidgets[SelectIndex].ToSharedRef());
					Panel->QuickItemWidgets.RemoveAt(SelectIndex);
					CurrentPathArray.RemoveAt(SelectIndex);
				}
			}
			for (const FString& MovePath : PathsToMove)
			{
				TargetPathArray.AddUnique(MovePath);
			}
			for (int32 i = 0; i < Panel->QuickItemWidgets.Num(); ++i)
			{
				Panel->QuickItemWidgets[i]->SetIndex(i);
				Panel->QuickItemWidgets[i]->SetSelected(false);
			}
			Panel->QuickPanelSelectedIndexes.Empty();
			Panel->FirstMenuSelectedIndex = -1;
			Panel->LastFirstMenuSelectedIndex = -1;
			FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
			Panel->OnRefreshClicked.ExecuteIfBound(NewTabName);
		});

		QuickItemWidgets.Add(TempQuickItemWidget);

		QuickPanelVertical->AddSlot()
		                  .AutoHeight()
		                  .HAlign(HAlign_Fill)
		[
			TempQuickItemWidget.ToSharedRef()
		];

		FSlateApplication::Get().PumpMessages();
	}
}

void SQuickPanelWidget::Refresh()
{
	if (QuickPanelVertical.IsValid())
	{
		QuickPanelVertical->ClearChildren();
	}
	QuickItemWidgets.Empty();
	QuickPanelSelectedIndexes.Empty();
	FirstMenuSelectedIndex = -1;
	LastFirstMenuSelectedIndex = -1;
	AddChildren(GetPathArray());
}

void SQuickPanelWidget::OnDragItem(const FVector2D Position, const float Offset, const int32 Index)
{
	if (!bIsDragging)
	{
		return;
	}
	if (DragLine.IsValid())
	{
		if (!bIsDragVisible)
		{
			if (FMath::Abs(Offset) > 5)
			{
				bIsDragVisible = true;
				DragLine->SetVisibility(EVisibility::SelfHitTestInvisible);
			}
		}

		const int DragIndex = FMath::Clamp(Position.Y / SQuickItemWidget::GetSize(), -1.f, QuickItemWidgets.Num() - 1.f);
		DragLine->SetRenderTransform(FSlateRenderTransform(FVector2D(0, DragIndex * SQuickItemWidget::GetSize())));
	}
}

void SQuickPanelWidget::OnDragItemStart(const FVector2D Position, const float Offset, const int32 Index)
{
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();
	if (IsShift || IsControl || IsAlt)
	{
		return;
	}
	bIsDragging = true;
	QuickPanelSelectedIndexes.Empty();
	QuickPanelSelectedIndexes.AddUnique(Index);
	for (int i = QuickItemWidgets.Num() - 1; i >= 0; --i)
	{
		QuickItemWidgets[i]->SetSelected(i == Index);
	}
}

void SQuickPanelWidget::OnDragItemEnd(const FVector2D Position, const float Offset, const int32 Index)
{
	if (!bIsDragging || !bIsDragVisible)
	{
		return;
	}
	bIsDragging = false;
	bIsDragVisible = false;
	DragLine->SetVisibility(EVisibility::Collapsed);

	TArray<FString>& PathArray = GetPathArray();

	if (QuickItemWidgets.Num() <= 1)
	{
		return;
	}
	if (!QuickItemWidgets.IsValidIndex(Index) || !PathArray.IsValidIndex(Index))
	{
		return;
	}
	const int OffsetInex = Offset / 30;

	const int NewDragIndex = FMath::Clamp(OffsetInex + Index, 0, QuickItemWidgets.Num() - 1);

	if (NewDragIndex == Index)
	{
		return;
	}

	const TSharedPtr<SQuickItemWidget> CurrentWidget = QuickItemWidgets[Index];
	QuickItemWidgets.RemoveAt(Index);
	QuickItemWidgets.Insert(CurrentWidget, NewDragIndex);
	const FString Path = PathArray[Index];
	PathArray.RemoveAt(Index);
	PathArray.Insert(Path, NewDragIndex);

	QuickPanelVertical->ClearChildren();
	for (int i = 0; i < QuickItemWidgets.Num(); ++i)
	{
		QuickItemWidgets[i]->SetIndex(i);

		QuickPanelVertical->AddSlot()
		                  .AutoHeight()
		                  .HAlign(HAlign_Fill)
		[
			QuickItemWidgets[i].ToSharedRef()
		];
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickPanelWidget::OnItemClick(const int32 Index)
{
	if (QuickItemWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		LastFirstMenuSelectedIndex = FirstMenuSelectedIndex;
	}
	if (!QuickItemWidgets.IsValidIndex(Index))
	{
		for (auto QuickItemWidget : QuickItemWidgets)
		{
			if (QuickItemWidget.IsValid())
			{
				if (QuickItemWidget->GetIsSelected())
				{
					QuickItemWidget->SetSelected(false);
				}
			}
		}
		FirstMenuSelectedIndex = -1;
		QuickPanelSelectedIndexes.Empty();
		return;
	}
	FirstMenuSelectedIndex = Index;
	const int32 Min = FMath::Min(FirstMenuSelectedIndex, LastFirstMenuSelectedIndex);
	const int32 Max = FMath::Max(FirstMenuSelectedIndex, LastFirstMenuSelectedIndex);
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();
	if (IsShift && !IsControl && !IsAlt)
	{
		for (int32 i = 0; i < QuickItemWidgets.Num(); ++i)
		{
			if (i >= Min && i <= Max)
			{
				QuickPanelSelectedIndexes.AddUnique(i);
			}
		}
	}
	else if (!IsShift && IsControl && !IsAlt)
	{
		QuickPanelSelectedIndexes.AddUnique(Index);
	}
	else
	{
		QuickPanelSelectedIndexes.Empty();
		QuickPanelSelectedIndexes.Add(FirstMenuSelectedIndex);
	}
	QuickPanelSelectedIndexes.Sort([](const int32& A, const int32& B)
	{
		return A > B;
	});

	for (int32 i = 0; i < QuickItemWidgets.Num(); ++i)
	{
		QuickItemWidgets[i]->SetSelected(QuickPanelSelectedIndexes.Contains(i));
	}
}

void SQuickPanelWidget::OnBrowseAssetClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickItemWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickItemWidgets[FirstMenuSelectedIndex]->BrowserToObject();
	}
}

bool SQuickPanelWidget::OnAssetRemoved(const FAssetData& AssetData)
{
	TArray<FString>& PathArray = GetPathArray();
	const int32 RemovedIndex = PathArray.Find(AssetData.ObjectPath.ToString());

	if (RemovedIndex < 0)
	{
		return false;
	}
	if (QuickPanelSelectedIndexes.IsValidIndex(RemovedIndex))
	{
		QuickPanelSelectedIndexes.Remove(RemovedIndex);
	}
	if (QuickItemWidgets.IsValidIndex(RemovedIndex))
	{
		PathArray.RemoveAt(RemovedIndex);
		QuickPanelVertical->RemoveSlot(QuickItemWidgets[RemovedIndex].ToSharedRef());
		QuickItemWidgets.RemoveAt(RemovedIndex);
	}

	for (int i = 0; i < QuickItemWidgets.Num(); ++i)
	{
		QuickItemWidgets[i]->SetIndex(i);
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	return true;
}

void SQuickPanelWidget::OnReferenceViewerClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickItemWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickItemWidgets[FirstMenuSelectedIndex]->ReferenceViewer();
	}
}

void SQuickPanelWidget::OnExploreFolderClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	for (const int32 Index : QuickPanelSelectedIndexes)
	{
		if (QuickItemWidgets.IsValidIndex(Index))
		{
			QuickItemWidgets[Index]->ExploreFolder();
		}
	}
}

void SQuickPanelWidget::OnClearAllFilesClicked()
{
	GetPathArray().Empty();
	QuickItemWidgets.Empty();
	QuickPanelVertical->ClearChildren();
	FirstMenuSelectedIndex = -1;
	LastFirstMenuSelectedIndex = -1;
	QuickPanelSelectedIndexes.Empty();
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickPanelWidget::OnSelectAllClicked()
{
	QuickPanelSelectedIndexes.Empty();

	for (int i = QuickItemWidgets.Num() - 1; i >= 0; --i)
	{
		QuickItemWidgets[i]->SetSelected(true);
		QuickPanelSelectedIndexes.Add(i);
	}
}

void SQuickPanelWidget::OnAddNewTabClicked() const
{
	AddNewTab.ExecuteIfBound();
}

void SQuickPanelWidget::OnRemoveTabClicked() const
{
	RemoveTab.ExecuteIfBound();
}

void SQuickPanelWidget::OnSaveClicked()
{
	for (const int32& TempSelectedIndex : QuickPanelSelectedIndexes)
	{
		if (QuickItemWidgets.IsValidIndex(TempSelectedIndex))
		{
			QuickItemWidgets[TempSelectedIndex]->Save();
		}
	}
}

void SQuickPanelWidget::OnSaveAllClicked()
{
	for (TSharedPtr<SQuickItemWidget> QuickItemWidget : QuickItemWidgets)
	{
		if (QuickItemWidget.IsValid())
		{
			QuickItemWidget->Save();
		}
	}
}

void SQuickPanelWidget::OnDeleteObject()
{
	if (QuickPanelSelectedIndexes.Num() <= 0)
	{
		return;
	}

	TArray<FString>& PathArray = GetPathArray();

	for (int i = 0; i < QuickPanelSelectedIndexes.Num(); ++i)
	{
		const int Index = QuickPanelSelectedIndexes[i];
		if (QuickItemWidgets.IsValidIndex(Index))
		{
			if (PathArray.IsValidIndex(Index))
			{
				PathArray.RemoveAt(Index);
			}
			QuickPanelVertical->RemoveSlot(QuickItemWidgets[Index].ToSharedRef());
			QuickItemWidgets.RemoveAt(Index);
		}
	}
	for (int i = 0; i < QuickItemWidgets.Num(); ++i)
	{
		QuickItemWidgets[i]->SetIndex(i);
	}

	QuickPanelSelectedIndexes.Empty();
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickPanelWidget::OnAddObjects(const TArray<FString>& NewPath, const int OffsetIndex)
{
	TArray<FString> CopyArray = GetPathArray();
	for (const FString& TempPath : NewPath)
	{
		if (!CopyArray.Contains(TempPath))
		{
			CopyArray.Add(TempPath);
		}
	}

	AddChildren(NewPath, OffsetIndex);
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickPanelWidget::RenameTab(const FString& OldName, const FString& NewName)
{
	if (TabName != OldName)
	{
		return;
	}
	TabName = NewName;
	for (auto QuickItemWidget : QuickItemWidgets)
	{
		if (QuickItemWidget.IsValid())
		{
			QuickItemWidget->RenameTab(OldName, NewName);
		}
	}
}

TSharedRef<SWidget> SQuickPanelWidget::GetObjThumbnailByPath(const FString& Path)
{
	const FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(FName(*Path));
	const TSharedPtr<FAssetThumbnail> AssetThumbnail = MakeShareable(new FAssetThumbnail(AssetData, SQuickItemWidget::GetIconSize(), SQuickItemWidget::GetIconSize(), nullptr));
	return AssetThumbnail->MakeThumbnailWidget();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
