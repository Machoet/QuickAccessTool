#include "PaginatedWidget.h"

#include "DoubleClickButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "EditorStyleSet.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

SPaginatedWidget::SPaginatedWidget()
	: LogicalSlots(this)
	  , VisualSlot(this)
	  , CombinedChildren(this)
{
	CombinedChildren.AddChildren(LogicalSlots);
	CombinedChildren.AddChildren(VisualSlot);
}

void SPaginatedWidget::Construct(const FArguments& InArgs)
{
	CurrentPageIndex = InArgs._InitialPageIndex;
	OnTabRename = InArgs._OnTabRename;

	if (CurrentPageIndex >= FQuickAccessToolModule::QuickAccessArchiveInfo.MultiPathMap.Num())
	{
		CurrentPageIndex = 0;
		FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	}
	SAssignNew(MainLayout, SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SAssignNew(TabButtonContainer, SHorizontalBox)
	]
	+ SVerticalBox::Slot()
	.FillHeight(1.0f)
	.Padding(0, 2.0f, 0, 0)
	[
		SAssignNew(ContentSwitcher, SWidgetSwitcher)
		.WidgetIndex(this, &SPaginatedWidget::GetActivePageIndex)
	];

#if ENGINE_MAJOR_VERSION >= 5
	{
		TUniquePtr<FSlot> NewVisualSlot = MakeUnique<FSlot>();
		(*NewVisualSlot)[MainLayout.ToSharedRef()];
		VisualSlot.AddSlot(FSlot::FSlotArguments(MoveTemp(NewVisualSlot)));
	}
#else
	{
		FSlot& NewVisualSlot = *new FSlot();
		VisualSlot.Add(&NewVisualSlot);
		NewVisualSlot[MainLayout.ToSharedRef()];
	}
#endif


	if (ContentSwitcher.IsValid())
	{
		for (int32 i = 0; i < InArgs.Slots.Num(); ++i)
		{
			FSlot& S = *InArgs.Slots[i];

#if ENGINE_MAJOR_VERSION >= 5
			TUniquePtr<FSlot> InternalSlot = MakeUnique<FSlot>();
			InternalSlot->Label(S.TabLabel);
			(*InternalSlot)[S.GetWidget()];
			LogicalSlots.AddSlot(FSlot::FSlotArguments(MoveTemp(InternalSlot)));
#else
			FSlot& InternalSlot = *new FSlot();
			InternalSlot.Label(S.TabLabel);
			InternalSlot[S.GetWidget()];
			LogicalSlots.Add(&InternalSlot);
#endif

			ContentSwitcher->AddSlot()[S.GetWidget()];
		}
	}

	RebuildTabButtons();
}

void SPaginatedWidget::AddPage(const FText& InLabel, const TSharedRef<SWidget>& InContent, bool bEnterEditor)
{
	if (ContentSwitcher.IsValid())
	{
#if ENGINE_MAJOR_VERSION >= 5
		TUniquePtr<FSlot> InternalSlot = MakeUnique<FSlot>();
		InternalSlot->Label(InLabel);
		(*InternalSlot)[InContent];
		LogicalSlots.AddSlot(FSlot::FSlotArguments(MoveTemp(InternalSlot)));
#else
		FSlot& InternalSlot = *new FSlot();
		InternalSlot.Label(InLabel);
		InternalSlot[InContent];
		LogicalSlots.Add(&InternalSlot);
#endif
		ContentSwitcher->AddSlot()[InContent];

		RebuildTabButtons(bEnterEditor);
	}
}

void SPaginatedWidget::RemovePage(int32 IndexToRemove)
{
	if (LogicalSlots.IsValidIndex(IndexToRemove) && ContentSwitcher.IsValid())
	{
		ContentSwitcher->RemoveSlot(LogicalSlots[IndexToRemove].GetWidget());

		LogicalSlots.RemoveAt(IndexToRemove);

		if (CurrentPageIndex >= LogicalSlots.Num())
		{
			CurrentPageIndex = FMath::Max(0, LogicalSlots.Num() - 1);
			FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveQuickPanelMenuIndex = CurrentPageIndex;
		}

		RebuildTabButtons();
	}
}

void SPaginatedWidget::ClearAllPages()
{
	LogicalSlots.Empty();
	TabButtonContainer->ClearChildren();

	if (ContentSwitcher.IsValid())
	{
		for (int32 i = ContentSwitcher->GetNumWidgets() - 1; i >= 0; --i)
		{
			ContentSwitcher->RemoveSlot(ContentSwitcher->GetWidget(i).ToSharedRef());
		}
	}

	CurrentPageIndex = 0;
	FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveQuickPanelMenuIndex = CurrentPageIndex;
	Invalidate(EInvalidateWidget::Layout);
}

TSharedPtr<SWidget> SPaginatedWidget::GetActivePageWidget() const
{
	if (ContentSwitcher.IsValid())
	{
		return ContentSwitcher->GetActiveWidget();
	}
	return nullptr;
}

void SPaginatedWidget::RebuildTabButtons(bool bEnterEditor)
{
	TabButtonContainer->ClearChildren();

	const FButtonStyle* TransparentButtonStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

	TWeakPtr<SPaginatedWidget> TempThis = SharedThis(this);
	for (int32 i = 0; i < LogicalSlots.Num(); ++i)
	{
		TSharedPtr<SInlineEditableTextBlock> InlineText;

		SAssignNew(InlineText, SInlineEditableTextBlock)
		.Text(LogicalSlots[i].TabLabel)
		.Font(this, &SPaginatedWidget::GetTabLabelFont, i)
		.ColorAndOpacity(this, &SPaginatedWidget::GetTabTextColor, i)
		.OnTextCommitted(this, &SPaginatedWidget::OnTabLabelCommitted, i)
		.IsSelected_Lambda([]() { return true; });

		TabButtonContainer->AddSlot()
		                  .AutoWidth()
		                  .Padding(2.0f, 0.0f)
		[
			SNew(SDoubleClickButton)
			.ButtonStyle(TransparentButtonStyle)
			.Cursor(EMouseCursor::Hand)
			.OnClicked_Lambda([TempThis, i]()
			{
				if (TempThis.IsValid())
				{
					if (TempThis.Pin()->CurrentPageIndex != i)
					{
						TempThis.Pin()->CurrentPageIndex = i;
						FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveQuickPanelMenuIndex = TempThis.Pin()->CurrentPageIndex;
						TempThis.Pin()->RebuildTabButtons();
						FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
					}
				}
				return FReply::Handled();
			})
			.OnDoubleClicked_Lambda([InlineText]()
			{
				if (InlineText.IsValid())
				{
					InlineText->EnterEditingMode();
				}
				return FReply::Handled();
			})
			.Content()
			[
				InlineText.ToSharedRef()
			]
		];
		if (bEnterEditor && i == LogicalSlots.Num() - 1)
		{
			if (InlineText.IsValid())
			{
				InlineText->EnterEditingMode();
			}
		}
	}
	Invalidate(EInvalidateWidget::Layout);
}

void SPaginatedWidget::OnTabLabelCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 Index)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FString OldID = LogicalSlots[Index].TabLabel.ToString();
		FString NewID = NewText.ToString();

		if (!NewID.IsEmpty() && OldID != NewID)
		{
			auto& PathMap = FQuickAccessToolModule::QuickAccessArchiveInfo.MultiPathMap;

			if (!PathMap.Contains(NewID))
			{
				LogicalSlots[Index].Label(NewText);
				if (PathMap.Contains(OldID))
				{
					TArray<FString> Data = PathMap[OldID];
					PathMap.Remove(OldID);
					PathMap.Add(NewID, Data);
					FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
					OnTabRename.ExecuteIfBound(OldID, NewID);
				}
			}
		}
	}

	RebuildTabButtons();
}

bool SPaginatedWidget::IsTabSelectedForEdit(int32 Index) const
{
	return CurrentPageIndex == Index;
}

void SPaginatedWidget::OnTabChanged(ECheckBoxState NewState, int32 Index)
{
	if (NewState == ECheckBoxState::Checked)
	{
		CurrentPageIndex = Index;
		FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveQuickPanelMenuIndex = CurrentPageIndex;
	}
}

FSlateColor SPaginatedWidget::GetTabTextColor(int32 Index) const
{
	return (CurrentPageIndex != Index)
		       ? FSlateColor(FLinearColor::White)
		       : FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
}

FSlateFontInfo SPaginatedWidget::GetTabLabelFont(int32 Index) const
{
	FSlateFontInfo FontInfo = FEditorStyle::Get().GetFontStyle("NormalText.Font");
	FontInfo.Size = 10;
	if (CurrentPageIndex == Index)
	{
		FontInfo.TypefaceFontName = TEXT("Bold");
	}
	else
	{
		FontInfo.TypefaceFontName = TEXT("Regular");
	}

	return FontInfo;
}

FChildren* SPaginatedWidget::GetChildren() { return &CombinedChildren; }

int32 SPaginatedWidget::GetActivePageIndex() const
{
	return CurrentPageIndex;
}

void SPaginatedWidget::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	if (VisualSlot.Num() > 0)
	{
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
			MainLayout.ToSharedRef(), FVector2D::ZeroVector, AllottedGeometry.GetLocalSize()
		));
	}
}

FVector2D SPaginatedWidget::ComputeDesiredSize(float) const { return MainLayout->GetDesiredSize(); }

ECheckBoxState SPaginatedWidget::IsTabChecked(int32 Index) const
{
	return (CurrentPageIndex == Index) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
