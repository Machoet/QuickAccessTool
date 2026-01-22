// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class IDetailLayoutBuilder;
class IPropertyHandle;
class SWidget;

struct FQuickAccessorItem
{
	FQuickAccessorItem(const FText& InText, const FName& InName)
		: Text(InText)
		, Name(InName)
	{
	}

	/** Text to display */
	FText Text;

	/** Name of the accessor */
	FName Name;
};

class QUICKACCESSTOOL_API SQuickCommonWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickCommonWidget)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	FReply OnPickColorClicked();
	
	void OnSelectedStopColorChanged(FLinearColor InNewColor);
	
	void OnCancelSelectedStopColorChange(FLinearColor PreviousColor);
	
	void CustomizeDetails();
	
	TSharedRef<SWidget> OnGenerateWidget( TSharedPtr<FQuickAccessorItem> InItem );

	void OnSelectionChanged(TSharedPtr<FQuickAccessorItem> InItem, ESelectInfo::Type InSelectionInfo);

	FText GetAccessorText() const;
private:
	TSharedPtr<SComboBox<TSharedPtr<FString>>> LanguageComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FQuickAccessorItem>>> CodeComboBox;
	
	TArray<TSharedPtr<FString>> LanguageOptions;
	
	TUniquePtr<FButtonStyle> ItemButtonStyle = nullptr;
	
	FLinearColor StartColor = FLinearColor::White;
	
	FLinearColor SelectColor = FLinearColor::White;
	
	TArray<TSharedPtr<FQuickAccessorItem>> Accessors;
	
	TUniquePtr<FTextBlockStyle> ColorButtonTextStyle;
};
