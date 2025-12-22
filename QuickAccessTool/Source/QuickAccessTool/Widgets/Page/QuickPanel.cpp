// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickPanel.h"
#include "SlateOptMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Brushes/SlateColorBrush.h"
#include "Misc/ScopedSlowTask.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "QuickAccessTool/Widgets/Sub/CustomItemObjectWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickPanel::Construct(const FArguments& InArgs)
{
	SAssignNew(QuickPanelVertical, SVerticalBox);

	FirstMenuSelectedIndex = -1;
	QuickAccessLineWidgets.Empty();

	AddChildren(FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray, true);

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
		.Padding(0.0f, SCustomItemObjectWidget::GetSize(), 0.f, 0.f)
		[
			DragLine.ToSharedRef()
		]
	];
}

void SQuickPanel::AddChildren(const TArray<FString>& InPathArray, const bool bIsInit)
{
	if (InPathArray.Num() <= 0)
	{
		return;
	}
	FScopedSlowTask SlowTask(InPathArray.Num(), QuickAccessToolLanguage::LoadAssert, true);
	SlowTask.MakeDialog();
	const int32 Offset = bIsInit ? 0 : FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Num();
	for (int32 Index = 0; Index < InPathArray.Num(); Index++)
	{
		const FString& Path = InPathArray[Index];

		SlowTask.EnterProgressFrame(1, FText::Format(QuickAccessToolLanguage::Loading, FText::FromString(Path)));

		if (SlowTask.ShouldCancel())
		{
			break;
		}

		const UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), nullptr, *Path);
		TSharedPtr<SCustomItemObjectWidget> QuickAccessLineWidget;
		SAssignNew(QuickAccessLineWidget, SCustomItemObjectWidget)
		.Path(Path)
		.Index(Index + Offset)
		.Text(FText::FromString(LoadedObj == nullptr ? "None" : LoadedObj->GetName()))
		.IconWidget(GetObjThumbnailByPath(Path))
		.OnItemDrag(this, &SQuickPanel::OnDragItem)
		.OnItemDragStart(this, &SQuickPanel::OnDragItemStart)
		.OnItemDragEnd(this, &SQuickPanel::OnDragItemEnd)
		.OnClicked(this, &SQuickPanel::OnItemClick)
		.OnClearAllClicked(this, &SQuickPanel::OnClearAllFilesClicked)
		.OnSelectAllClicked(this, &SQuickPanel::OnSelectAllClicked);

		QuickAccessLineWidgets.Add(QuickAccessLineWidget);

		QuickPanelVertical->AddSlot()
		                  .AutoHeight()
		                  .HAlign(HAlign_Fill)
		[
			QuickAccessLineWidget.ToSharedRef()
		];

		FSlateApplication::Get().PumpMessages();
	}
}

void SQuickPanel::OnDragItem(const FVector2D Position, const float Offset, const int32 Index)
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

		const int DragIndex = FMath::Clamp(Position.Y / SCustomItemObjectWidget::GetSize(), -1.f, QuickAccessLineWidgets.Num() - 1.f);
		DragLine->SetRenderTransform(FSlateRenderTransform(FVector2D(0, DragIndex * SCustomItemObjectWidget::GetSize())));
	}
}

void SQuickPanel::OnDragItemStart(const FVector2D Position, const float Offset, const int32 Index)
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
	for (int i = QuickAccessLineWidgets.Num() - 1; i >= 0; --i)
	{
		QuickAccessLineWidgets[i]->SetSelected(i == Index);
	}
}

void SQuickPanel::OnDragItemEnd(const FVector2D Position, const float Offset, const int32 Index)
{
	if (!bIsDragging || !bIsDragVisible)
	{
		return;
	}
	bIsDragging = false;
	bIsDragVisible = false;
	DragLine->SetVisibility(EVisibility::Collapsed);
	if (QuickAccessLineWidgets.Num() <= 1)
	{
		return;
	}
	if (!QuickAccessLineWidgets.IsValidIndex(Index) || !FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.IsValidIndex(Index))
	{
		return;
	}
	const int OffsetInex = Offset / 30;

	const int NewDragIndex = FMath::Clamp(OffsetInex + Index, 0, QuickAccessLineWidgets.Num() - 1);

	if (NewDragIndex == Index)
	{
		return;
	}

	const TSharedPtr<SCustomItemObjectWidget> CurrentWidget = QuickAccessLineWidgets[Index];
	QuickAccessLineWidgets.RemoveAt(Index);
	QuickAccessLineWidgets.Insert(CurrentWidget, NewDragIndex);
	const FString Path = FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray[Index];
	FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.RemoveAt(Index);
	FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Insert(Path, NewDragIndex);

	QuickPanelVertical->ClearChildren();
	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);

		QuickPanelVertical->AddSlot()
		                  .AutoHeight()
		                  .HAlign(HAlign_Fill)
		[
			QuickAccessLineWidgets[i].ToSharedRef()
		];
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickPanel::OnItemClick(const int32 Index)
{
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		LastFirstMenuSelectedIndex = FirstMenuSelectedIndex;
	}
	if (!QuickAccessLineWidgets.IsValidIndex(Index))
	{
		for (auto QuickAccessLineWidget : QuickAccessLineWidgets)
		{
			if (QuickAccessLineWidget.IsValid())
			{
				if (QuickAccessLineWidget->GetIsSelected())
				{
					QuickAccessLineWidget->SetSelected(false);
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
		for (int32 i = 0; i < QuickAccessLineWidgets.Num(); ++i)
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

	for (int32 i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetSelected(QuickPanelSelectedIndexes.Contains(i));
	}
}

void SQuickPanel::OnBrowseAssetClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickAccessLineWidgets[FirstMenuSelectedIndex]->BrowserToObject();
	}
}

bool SQuickPanel::OnAssetRemoved(const FAssetData& AssetData)
{
	const int32 RemovedIndex = FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Find(AssetData.ObjectPath.ToString());
	if (RemovedIndex < 0)
	{
		return false;
	}
	if (QuickPanelSelectedIndexes.IsValidIndex(RemovedIndex))
	{
		QuickPanelSelectedIndexes.Remove(RemovedIndex);
	}
	if (QuickAccessLineWidgets.IsValidIndex(RemovedIndex))
	{
		FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.RemoveAt(RemovedIndex);
		QuickPanelVertical->RemoveSlot(QuickAccessLineWidgets[RemovedIndex].ToSharedRef());
		QuickAccessLineWidgets.RemoveAt(RemovedIndex);
	}

	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);
	}
	return true;
}

void SQuickPanel::OnReferenceViewerClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickAccessLineWidgets[FirstMenuSelectedIndex]->ReferenceViewer();
	}
}

void SQuickPanel::OnExploreFolderClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	for (const int32 Index : QuickPanelSelectedIndexes)
	{
		if (QuickAccessLineWidgets.IsValidIndex(Index))
		{
			QuickAccessLineWidgets[Index]->ExploreFolder();
		}
	}
}

void SQuickPanel::OnClearAllFilesClicked()
{
	QuickAccessLineWidgets.Empty();
	QuickPanelVertical->ClearChildren();
	FirstMenuSelectedIndex = -1;
	LastFirstMenuSelectedIndex = -1;
	QuickPanelSelectedIndexes.Empty();
}

void SQuickPanel::OnSelectAllClicked()
{
	QuickPanelSelectedIndexes.Empty();

	for (int i = QuickAccessLineWidgets.Num() - 1; i >= 0; --i)
	{
		QuickAccessLineWidgets[i]->SetSelected(true);
		QuickPanelSelectedIndexes.Add(i);
	}
}

void SQuickPanel::OnSaveClicked()
{
	for (const int32& TempSelectedIndex : QuickPanelSelectedIndexes)
	{
		if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
		{
			QuickAccessLineWidgets[TempSelectedIndex]->Save();
		}
	}
}

void SQuickPanel::OnSaveAllClicked()
{
	for (TSharedPtr<SCustomItemObjectWidget> QuickAccessLineWidget : QuickAccessLineWidgets)
	{
		if (QuickAccessLineWidget.IsValid())
		{
			QuickAccessLineWidget->Save();
		}
	}
}

void SQuickPanel::OnDeleteObject()
{
	if (QuickPanelSelectedIndexes.Num() <= 0)
	{
		return;
	}
	for (int i = 0; i < QuickPanelSelectedIndexes.Num(); ++i)
	{
		const int Index = QuickPanelSelectedIndexes[i];
		if (QuickAccessLineWidgets.IsValidIndex(Index))
		{
			FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.RemoveAt(Index);
			QuickPanelVertical->RemoveSlot(QuickAccessLineWidgets[Index].ToSharedRef());
			QuickAccessLineWidgets.RemoveAt(Index);
		}
	}
	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);
	}

	QuickPanelSelectedIndexes.Empty();
}

void SQuickPanel::OnAddObjects(const TArray<FString>& NewPath)
{
	AddChildren(NewPath);
}

TSharedRef<SWidget> SQuickPanel::GetObjThumbnailByPath(const FString& Path)
{
	const FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(FName(*Path));
	const TSharedPtr<FAssetThumbnail> AssetThumbnail = MakeShareable(new FAssetThumbnail(AssetData, SCustomItemObjectWidget::GetIconSize(), SCustomItemObjectWidget::GetIconSize(), nullptr));
	return AssetThumbnail->MakeThumbnailWidget();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
