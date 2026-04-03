// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickCommonWidget.h"
#include "DetailLayoutBuilder.h"
#include "EditorStyleSet.h"
#include "ISourceCodeAccessModule.h"
#include "ISourceCodeAccessor.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateColorBrush.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GameFramework/InputSettings.h"
#include "Internationalization/Culture.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Windows/WindowsPlatformApplicationMisc.h"


#define LOCTEXT_NAMESPACE "SourceCodeAccessSettingsDetails"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickCommonWidget::Construct(const FArguments& InArgs)
{
	auto AddLanguage = [&](const FString& Code, const FString& DisplayName)
	{
		LanguageOptions.Add(MakeShared<FString>(Code));
	};

	AddLanguage("en", TEXT("English"));
	AddLanguage("zh-Hans", TEXT("简体中文"));
	AddLanguage("ja", TEXT("日本語"));
	AddLanguage("ko", TEXT("한국어"));
	AddLanguage("fr", TEXT("Français"));
	AddLanguage("de", TEXT("Deutsch"));
	AddLanguage("es", TEXT("Español"));
	LanguageComboBox = SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&LanguageOptions)
		.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& CultureCode)
		{
			FString DisplayName;
			if (*CultureCode == "en") DisplayName = TEXT("English");
			else if (*CultureCode == "zh-Hans") DisplayName = TEXT("简体中文");
			else if (*CultureCode == "ja") DisplayName = TEXT("日本語");
			else if (*CultureCode == "ko") DisplayName = TEXT("한국어");
			else if (*CultureCode == "fr") DisplayName = TEXT("Français");
			else if (*CultureCode == "de") DisplayName = TEXT("Deutsch");
			else if (*CultureCode == "es") DisplayName = TEXT("Español");
			else DisplayName = *CultureCode;

			return SNew(STextBlock).Text(FText::FromString(DisplayName));
		})
		.OnSelectionChanged_Lambda([](const TSharedPtr<FString>& NewCulture, ESelectInfo::Type)
		{
			if (NewCulture.IsValid())
			{
				if (FInternationalization::Get().SetCurrentCulture(*NewCulture))
				{
					const FString ConfigFilePath = GEditorSettingsIni;
					GConfig->SetString(TEXT("Internationalization"), TEXT("Language"), **NewCulture, *ConfigFilePath);
					GConfig->SetString(TEXT("Internationalization"), TEXT("Locale"), **NewCulture, *ConfigFilePath);
					GConfig->Flush(false, *ConfigFilePath);
				}
			}
		})
		.Content()
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				const FString CurrentCulture = FInternationalization::Get().GetCurrentCulture()->GetName();
				if (CurrentCulture == "en") return FText::FromString(TEXT("English"));
				if (CurrentCulture == "zh-Hans") return FText::FromString(TEXT("简体中文"));
				return FText::FromString(CurrentCulture);
			})
		];

	ItemButtonStyle = MakeUnique<FButtonStyle>();

#if ENGINE_MAJOR_VERSION >= 5
	ItemButtonStyle->SetNormal(FSlateColorBrush(FLinearColor(0, 0, 0, 1)));
	ItemButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.65)));
	ItemButtonStyle->SetPressed(FSlateColorBrush(FLinearColor(0, 0, 0, 1)));
	ItemButtonStyle->SetNormalPadding(FMargin(2, 2, 2, 2));
	ItemButtonStyle->SetPressedPadding(FMargin(2, 2, 2, 2));
	FTextBlockStyle TempColorButtonTextStyle = FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("FlatButton.DefaultTextStyle");
	TempColorButtonTextStyle.Font.Size = 8;
	ColorButtonTextStyle = MakeUnique<FTextBlockStyle>(TempColorButtonTextStyle);
#else
	ItemButtonStyle->SetNormal(FSlateColorBrush(FLinearColor(0.7f, 0.7, 0.7, 1)));
	ItemButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1.0f, 0.4, 0, 0.65)));
	ItemButtonStyle->SetPressed(FSlateColorBrush(FLinearColor(0.7, 0.7, 0.7, 1)));
	ItemButtonStyle->SetNormalPadding(FMargin(2, 2, 2, 2));
	ItemButtonStyle->SetPressedPadding(FMargin(2, 2, 2, 2));
	FTextBlockStyle TempColorButtonTextStyle = FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("FlatButton.DefaultTextStyle");
	TempColorButtonTextStyle.ColorAndOpacity = FLinearColor(0.f, 0.f, 0.f, 1.f);
	TempColorButtonTextStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 8));
	TempColorButtonTextStyle.SetHighlightColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.f));
	TempColorButtonTextStyle.SetShadowColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.f));
	ColorButtonTextStyle = MakeUnique<FTextBlockStyle>(TempColorButtonTextStyle);
#endif
	CustomizeDetails();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(QuickAccessToolLanguage::Language)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				LanguageComboBox.ToSharedRef()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(QuickAccessToolLanguage::SourceCodeEditor)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				CodeComboBox.ToSharedRef()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(QuickAccessToolLanguage::CopyColorToClipboard)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Padding(5, 0)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]
					{
						return FQuickAccessToolModule::QuickAccessArchiveInfo.bCopyColorToClipboard
							       ? ECheckBoxState::Checked
							       : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InCheckBoxState)
					{
						FQuickAccessToolModule::QuickAccessArchiveInfo.bCopyColorToClipboard = InCheckBoxState ==
							ECheckBoxState::Checked;
						FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
					})
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.HeightOverride(20)
				[
					SNew(SButton)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.ButtonStyle(ItemButtonStyle.Get())
					.TextStyle(ColorButtonTextStyle.Get())
					.Text(QuickAccessToolLanguage::OpenColorPicker)
					.OnClicked_Raw(this, &SQuickCommonWidget::OnPickColorClicked)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(QuickAccessToolLanguage::LessCPU)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([ ]
				{
					const UEditorPerformanceSettings* Settings = GetMutableDefault<
						UEditorPerformanceSettings>();
					return (Settings->bThrottleCPUWhenNotForeground
						        ? ECheckBoxState::Checked
						        : ECheckBoxState::Unchecked);
				})
				.OnCheckStateChanged_Lambda([](const ECheckBoxState InCheckBoxState)
				{
					UEditorPerformanceSettings* Settings = GetMutableDefault<
						UEditorPerformanceSettings>();
					Settings->bThrottleCPUWhenNotForeground = InCheckBoxState ==
						ECheckBoxState::Checked;
					Settings->SaveConfig();
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(QuickAccessToolLanguage::UseMouseForTouch)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([ ]
				{
					const UInputSettings* Settings = GetMutableDefault<
						UInputSettings>();
					return (Settings->bUseMouseForTouch
						        ? ECheckBoxState::Checked
						        : ECheckBoxState::Unchecked);
				})
				.OnCheckStateChanged_Lambda([](const ECheckBoxState InCheckBoxState)
				{
					UInputSettings* Settings = GetMutableDefault<
						UInputSettings>();
					Settings->bUseMouseForTouch = InCheckBoxState ==
						ECheckBoxState::Checked;
					Settings->SaveConfig();
				})
			]
		]
	];
}


FReply SQuickCommonWidget::OnPickColorClicked()
{
	StartColor = SelectColor;
	FColorPickerArgs ColorPickerArgs;
	ColorPickerArgs.bOnlyRefreshOnMouseUp = true;
	ColorPickerArgs.bIsModal = false;
	ColorPickerArgs.ParentWidget = SharedThis(this);
	ColorPickerArgs.bUseAlpha = false;
	ColorPickerArgs.InitialColorOverride = SelectColor;
	ColorPickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SQuickCommonWidget::OnSelectedStopColorChanged);
	ColorPickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SQuickCommonWidget::OnCancelSelectedStopColorChange);
	OpenColorPicker(ColorPickerArgs);
	return FReply::Handled();
}


void SQuickCommonWidget::OnSelectedStopColorChanged(const FLinearColor InNewColor)
{
	SelectColor = InNewColor;
	if (FQuickAccessToolModule::QuickAccessArchiveInfo.bCopyColorToClipboard)
	{
		const FString ColorString = FString::Printf(
			TEXT("R: %f, G: %f, B: %f, A: %f"), SelectColor.R, SelectColor.G, SelectColor.B, SelectColor.A);
		FPlatformApplicationMisc::ClipboardCopy(*ColorString);
	}
}

void SQuickCommonWidget::OnCancelSelectedStopColorChange(FLinearColor PreviousColor)
{
	SelectColor = StartColor;
}

void SQuickCommonWidget::CustomizeDetails()
{
	Accessors.Empty();

	const int32 FeatureCount = IModularFeatures::Get().GetModularFeatureImplementationCount("SourceCodeAccessor");
	for (int32 FeatureIndex = 0; FeatureIndex < FeatureCount; FeatureIndex++)
	{
		IModularFeature* Feature = IModularFeatures::Get().GetModularFeatureImplementation("SourceCodeAccessor", FeatureIndex);
		check(Feature);

		const ISourceCodeAccessor& Accessor = *static_cast<ISourceCodeAccessor*>(Feature);
		if (Accessor.GetFName() != FName("None"))
		{
			Accessors.Add(MakeShareable(new FQuickAccessorItem(Accessor.GetNameText(), Accessor.GetFName())));
		}
	}

	SAssignNew(CodeComboBox, SComboBox<TSharedPtr<FQuickAccessorItem>>)
	.ToolTipText(LOCTEXT("PreferredAccessorToolTip", "Choose the way to access source code."))
	.OptionsSource(&Accessors)
	.OnSelectionChanged(this, &SQuickCommonWidget::OnSelectionChanged)
	.ContentPadding(2.f)
	.OnGenerateWidget(this, &SQuickCommonWidget::OnGenerateWidget)
	.Content()
	[
		SNew(STextBlock)
		.Text(this, &SQuickCommonWidget::GetAccessorText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
}

TSharedRef<SWidget> SQuickCommonWidget::OnGenerateWidget(TSharedPtr<FQuickAccessorItem> InItem)
{
	return SNew(STextBlock)
		.Text(InItem->Text);
}

void SQuickCommonWidget::OnSelectionChanged(TSharedPtr<FQuickAccessorItem> InItem, ESelectInfo::Type InSelectionInfo)
{
	GConfig->SetString(TEXT("/Script/SourceCodeAccess.SourceCodeAccessSettings"), TEXT("PreferredAccessor"), *InItem->Name.ToString(), GEditorSettingsIni);
	ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>("SourceCodeAccess");
	SourceCodeAccessModule.SetAccessor(InItem->Name);

}

FText SQuickCommonWidget::GetAccessorText() const
{
	ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>("SourceCodeAccess");
	return SourceCodeAccessModule.GetAccessor().GetNameText();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


#undef LOCTEXT_NAMESPACE
