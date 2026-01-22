#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SPanel.h"
#include "Layout/Children.h"
#include "SlotBase.h"
#include "Widgets/Layout/SWidgetSwitcher.h"


DECLARE_DELEGATE_TwoParams(FOnTabRename, const FString,  const FString);

class SPaginatedWidget : public SPanel
{
public:
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot() : TSlotBase<FSlot>(), TabLabel(FText::GetEmpty())
		{
		}

		FSlot& Label(const FText& InText)
		{
			TabLabel = InText;
			return *this;
		}

		FText TabLabel;

		FSlot& operator[](TSharedRef<SWidget> InWidget)
		{
			TSlotBase<FSlot>::operator[](InWidget);
			return *this;
		}
	};

	static FSlot& Slot() { return *(new FSlot()); }

	SLATE_BEGIN_ARGS(SPaginatedWidget)
			: _InitialPageIndex(0)
		{
		}

		SLATE_ARGUMENT(int32, InitialPageIndex)
		SLATE_EVENT(FOnTabRename, OnTabRename)
		SLATE_SUPPORTS_SLOT(SPaginatedWidget::FSlot)
	SLATE_END_ARGS()

	SPaginatedWidget();

	void Construct(const FArguments& InArgs);

	void AddPage(const FText& InLabel, const TSharedRef<SWidget>& InContent, bool bEnterEditor = false);

	void RemovePage(int32 IndexToRemove);

	void ClearAllPages();

	TSharedPtr<SWidget> GetActivePageWidget() const;

	template <typename T>
	TSharedPtr<T> GetActivePageWidgetAs() const
	{
		const TSharedPtr<SWidget> ActiveWidget = GetActivePageWidget();
		return ActiveWidget.IsValid() ? StaticCastSharedPtr<T>(ActiveWidget) : nullptr;
	}

	template <typename T>
	void GetAllPageWidgetsAs(TArray<TSharedPtr<T>>& OutPages) const
	{
		if (ContentSwitcher.IsValid())
		{
			const int Num = ContentSwitcher->GetNumWidgets();
			for (int32 i = 0; i < Num; ++i)
			{
				TSharedPtr<SWidget> Widget = ContentSwitcher->GetWidget(i);
				if (Widget.IsValid())
				{
					OutPages.Add(StaticCastSharedPtr<T>(Widget));
				}
			}
		}
	}

	int32 GetActivePageIndex() const;

protected:
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;

private:
	void RebuildTabButtons(bool bEnterEditor = false);

	bool IsTabSelectedForEdit(int32 Index) const;

	void OnTabLabelCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 Index);

	ECheckBoxState IsTabChecked(int32 Index) const;

	void OnTabChanged(ECheckBoxState NewState, int32 Index);

	FSlateColor GetTabTextColor(int32 Index) const;

	FSlateFontInfo GetTabLabelFont(int32 Index) const;

	TPanelChildren<FSlot> LogicalSlots;

	TPanelChildren<FSlot> VisualSlot;

	FCombinedChildren CombinedChildren;

	TSharedPtr<SVerticalBox> MainLayout = nullptr;

	TSharedPtr<SHorizontalBox> TabButtonContainer = nullptr;

	TSharedPtr<SWidgetSwitcher> ContentSwitcher = nullptr;

	FOnTabRename OnTabRename;
	
	int32 CurrentPageIndex = 0;
};
