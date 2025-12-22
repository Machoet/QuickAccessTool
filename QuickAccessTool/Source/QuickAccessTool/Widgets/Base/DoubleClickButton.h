// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class QUICKACCESSTOOL_API SDoubleClickButton : public SButton
{
public:
	SLATE_BEGIN_ARGS(SDoubleClickButton)
		{
		}

		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)

		SLATE_EVENT(FOnClicked, OnClicked)
		
		SLATE_EVENT(FOnClicked, OnDoubleClicked)
		
		SLATE_EVENT(FSimpleDelegate, OnPressed)
		
		SLATE_EVENT(FSimpleDelegate, OnReleased)


	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void Press() override;
	
	virtual void Release() override;

private:	
	FOnClicked OnDoubleClicked;
	FSimpleDelegate OnQuickPressed;
	FSimpleDelegate OnQuickReleased;
};
