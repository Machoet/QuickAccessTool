// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class QUICKACCESSTOOL_API SQuickTaskWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickTaskWidget)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void OnTaskTextChanged(const FText& NewText);
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	
	TSharedPtr<class SMultiLineEditableText> CustomTaskMultiLineEditableText = nullptr;
};
