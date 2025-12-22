// Fill out your copyright notice in the Description page of Project Settings.


#include "DoubleClickButton.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDoubleClickButton::Construct(const FArguments& InArgs)
{
	SetOnClicked(InArgs._OnClicked);
	SetButtonStyle(InArgs._ButtonStyle);
	OnDoubleClicked = InArgs._OnDoubleClicked;
	OnQuickPressed = InArgs._OnPressed;
	OnQuickReleased = InArgs._OnReleased;
}

FReply SDoubleClickButton::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (OnDoubleClicked.IsBound())
	{
		FReply Reply = OnDoubleClicked.Execute();
		return Reply;
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
