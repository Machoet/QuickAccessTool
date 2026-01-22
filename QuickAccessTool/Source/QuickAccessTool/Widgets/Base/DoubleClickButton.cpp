#include "DoubleClickButton.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDoubleClickButton::Construct(const FArguments& InArgs)
{
	OnDoubleClicked = InArgs._OnDoubleClicked;
	OnQuickPressed = InArgs._OnPressed;
	OnQuickReleased = InArgs._OnReleased;

	SButton::Construct(SButton::FArguments()
		.ButtonStyle(InArgs._ButtonStyle)
		.OnClicked(InArgs._OnClicked)
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.Content()
		[
			InArgs._Content.Widget
		]
	);
}

FReply SDoubleClickButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (OnDoubleClicked.IsBound())
	{
		return OnDoubleClicked.Execute();
	}
	return SButton::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

void SDoubleClickButton::Press()
{
	if (!IsPressed())
	{
		OnQuickPressed.ExecuteIfBound();
	}
	SButton::Press();
}

void SDoubleClickButton::Release()
{
	if (IsPressed())
	{
		OnQuickReleased.ExecuteIfBound();
	}
	SButton::Release();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION