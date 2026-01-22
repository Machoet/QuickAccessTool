// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SButton.h"
#include "Framework/SlateDelegates.h"

class QUICKACCESSTOOL_API SDoubleClickButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SDoubleClickButton)
			: _Content()
			  , _ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
			  , _OnClicked()
			  , _OnDoubleClicked()
			  , _HAlign(HAlign_Fill)
			  , _VAlign(VAlign_Fill)
		{
		}

		SLATE_DEFAULT_SLOT(FArguments, Content)

		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)

		SLATE_EVENT(FOnClicked, OnClicked)

		SLATE_EVENT(FOnClicked, OnDoubleClicked)

		SLATE_EVENT(FSimpleDelegate, OnPressed)
		SLATE_EVENT(FSimpleDelegate, OnReleased)

		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void Press() override;

	virtual void Release() override;

private:
	FOnClicked OnDoubleClicked;
	FSimpleDelegate OnQuickPressed;
	FSimpleDelegate OnQuickReleased;
};
