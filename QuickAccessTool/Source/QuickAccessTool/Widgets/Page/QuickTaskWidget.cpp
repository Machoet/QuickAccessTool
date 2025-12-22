// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickTaskWidget.h"

#include "SlateOptMacros.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "Widgets/Text/SMultiLineEditableText.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickTaskWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(CustomTaskMultiLineEditableText, SMultiLineEditableText)
		.AutoWrapText(true)
		.Font_Lambda([this]()
		{
			return FCoreStyle::GetDefaultFontStyle("Regular", FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskFontSize);
		})
		.Text(FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskText.IsEmpty()
			      ? QuickAccessToolLanguage::AddNewTask
			      : FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskText)
		.OnTextChanged_Raw(this, &SQuickTaskWidget::OnTaskTextChanged)
	];
}

void SQuickTaskWidget::OnTaskTextChanged(const FText& NewText)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskText = NewText;
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

FReply SQuickTaskWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
	if (!MenuTexts.IsValidIndex(FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex) || FMath::IsNearlyEqual(MouseEvent.GetWheelDelta(), 0))
	{
		return Reply;
	}

	if (MenuTexts[FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex].ToString() == QuickAccessToolLanguage::CustomTask.ToString())
	{
		FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskFontSize = FMath::Clamp(
			static_cast<int32>(FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskFontSize + MouseEvent.GetWheelDelta()), 5, 30);
		CustomTaskMultiLineEditableText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", FQuickAccessToolModule::QuickAccessArchiveInfo.CustomTaskFontSize));
		FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
		return FReply::Handled();
	}
	return Reply;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
