// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickAccessWidget.h"
#include "QuickAccessLineWidget.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Internationalization/Culture.h"
#include "QuickAccessTool/Common/QuickAccessLibrary.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Windows/WindowsPlatformApplicationMisc.h"
#if ENGINE_MAJOR_VERSION == 4
#include "Brushes/SlateColorBrush.h"
#include "Misc/ScopedSlowTask.h"
#elif ENGINE_MAJOR_VERSION == 5
#include "EditorStyleSet.h"
#include "Misc/ScopedSlowTask.h"
#endif
#include "SlateOptMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SScrollBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickAccessWidget::Construct(const FArguments& InArgs)
{
	QuickAccessArchiveInfo.Load();
	SAssignNew(QuickAccessLineVertical, SVerticalBox);
	QuickAccessLineWidgets.Empty();
	FirstMenuSelectedIndex = -1;
	AddChildren(QuickAccessArchiveInfo.PathArray, true);
	OnAddObjectClicked = InArgs._OnAddObjectClicked;

	TWeakPtr<SQuickAccessWidget> TempThis = SharedThis(this);
	ChordFunctionMap.Add(FInputChord(EKeys::Delete, true, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnClearAllFilesClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::Delete, false, false, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnDeleteObject();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::B, false, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnBrowseAssetClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::R, true, false, true, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnReferenceViewerClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::Q, true, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnExploreFolderClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::A, false, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnSelectAllClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::S, false, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnSaveClicked();
		}
	});
	ChordFunctionMap.Add(FInputChord(EKeys::S, true, true, false, false), [TempThis]()
	{
		if (TempThis.IsValid())
		{
			TempThis.Pin()->OnSaveAllClicked();
		}
	});

	ChordFunctionMap.Add(FInputChord(EKeys::V, false, false, true, false), [TempThis]()
	{
		UQuickAccessLibrary::SaveClipboardToAsset();
	});

	SlateColorBrush = MakeUnique<FSlateColorBrush>(FLinearColor::Yellow);
	SlateColorBrush->SetImageSize(FVector2D(1, 1));

	SAssignNew(DragLine, SImage)
	.Visibility(EVisibility::Collapsed)
	.Image(SlateColorBrush.Get());

	TitleButtonStyle = MakeUnique<FButtonStyle>();
	TitleButtonStyle->SetNormal(FSlateNoResource());
	TitleButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.65)));
	TitleButtonStyle->SetPressed(FSlateNoResource());
	TitleButtonStyle->SetNormalPadding(FMargin(2, 2, 2, 2));
	TitleButtonStyle->SetPressedPadding(FMargin(2, 2, 2, 2));

	ItemButtonStyle = MakeUnique<FButtonStyle>();
	ItemButtonStyle->SetNormal(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.2)));
	ItemButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.65)));
	ItemButtonStyle->SetPressed(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.4)));
	ItemButtonStyle->SetNormalPadding(FMargin(2, 2, 2, 2));
	ItemButtonStyle->SetPressedPadding(FMargin(2, 2, 2, 2));

	TitleBlockStyle = MakeUnique<FTextBlockStyle>();
	TitleBlockStyle->ColorAndOpacity = FLinearColor::White;

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

	SAssignNew(MenuHorizontalBox, SHorizontalBox);
	for (int32 i = 0; i < MenuTexts.Num(); i++)
	{
		TSharedPtr<SButton> MenuButton;
		SAssignNew(MenuButton, SButton)
		.ButtonStyle(TitleButtonStyle.Get())
		.Text(MenuTexts[i])
		.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
		.OnClicked(this, &SQuickAccessWidget::OnMenuClicked, i);

		MenuHorizontalBox->AddSlot()
		                 .HAlign(HAlign_Right)
		                 .AutoWidth()
		[
			MenuButton.ToSharedRef()
		];
		MenuButtons.Add(MenuButton);
		if (QuickAccessArchiveInfo.ActiveMenuIndex == i)
		{
			MenuButton->SetEnabled(false);
		}
	}

	ChildSlot
	[
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(2.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MenuHorizontalBox.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
				.Orientation(Orient_Horizontal)
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				+ SScrollBox::Slot()
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(this, &SQuickAccessWidget::GetMenuWidgetIndex)
					+ SWidgetSwitcher::Slot()
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Fill)
						[
							QuickAccessLineVertical.ToSharedRef()
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Fill)
						.Padding(0.0f, SQuickAccessLineWidget::GetSize() - SQuickAccessLineWidget::GetOffset(), 0.f, 0.f)
						[
							DragLine.ToSharedRef()
						]
					]
					+ SWidgetSwitcher::Slot()
					.Padding(4.0f, 0, 0, 0)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.Text(QuickAccessToolLanguage::Language)
							]
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								LanguageComboBox.ToSharedRef()
							]
						]
						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.Text(QuickAccessToolLanguage::LessCPU)
							]
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
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
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(SButton)
								.ButtonStyle(ItemButtonStyle.Get())
								.Text(QuickAccessToolLanguage::OpenColorPicker)
								.OnClicked_Raw(this, &SQuickAccessWidget::OnPickColorClicked)
							]
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Fill)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Left)
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.ToolTipText(QuickAccessToolLanguage::CopyColorToClipboard)
									.IsChecked_Lambda([this]
									{
										return QuickAccessArchiveInfo.bCopyColorToClipboard
											       ? ECheckBoxState::Checked
											       : ECheckBoxState::Unchecked;
									})
									.OnCheckStateChanged_Lambda([this](const ECheckBoxState InCheckBoxState)
									{
										QuickAccessArchiveInfo.bCopyColorToClipboard = InCheckBoxState ==
											ECheckBoxState::Checked;
										QuickAccessArchiveInfo.Save();
									})
								]
							]
						]
					]
					+ SWidgetSwitcher::Slot()
					.Padding(4.0f, 0, 0, 0)
					[
						SAssignNew(CustomTaskMultiLineEditableText, SMultiLineEditableText)
						.AutoWrapText(true)
						.Font_Lambda([this]()
						{
							return FCoreStyle::GetDefaultFontStyle("Regular", QuickAccessArchiveInfo.CustomTaskFontSize);
						})
						.Text(QuickAccessArchiveInfo.CustomTaskText.IsEmpty() ? QuickAccessToolLanguage::AddNewTask : QuickAccessArchiveInfo.CustomTaskText)
						.OnTextChanged_Raw(this, &SQuickAccessWidget::OnTaskTextChanged)
					]
				]
			]
		]
	];

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");

	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &SQuickAccessWidget::OnAssetRemoved);
}

void SQuickAccessWidget::OnTaskTextChanged(const FText& NewText)
{
	QuickAccessArchiveInfo.CustomTaskText = NewText;
	QuickAccessArchiveInfo.Save();
}

void SQuickAccessWidget::OnAssetRemoved(const FAssetData& AssetData)
{
	const int32 RemovedIndex = QuickAccessArchiveInfo.PathArray.Find(AssetData.ObjectPath.ToString());
	if (RemovedIndex < 0)
	{
		return;
	}
	if (FirstMenuSelectedIndexes.IsValidIndex(RemovedIndex))
	{
		FirstMenuSelectedIndexes.Remove(RemovedIndex);
	}
	if (QuickAccessLineWidgets.IsValidIndex(RemovedIndex))
	{
		QuickAccessArchiveInfo.PathArray.RemoveAt(RemovedIndex);
		QuickAccessLineVertical->RemoveSlot(QuickAccessLineWidgets[RemovedIndex].ToSharedRef());
		QuickAccessLineWidgets.RemoveAt(RemovedIndex);
	}

	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);
	}
	QuickAccessArchiveInfo.Save();
}

void SQuickAccessWidget::AddChildren(TArray<FString> InPathArray, const bool bIsInit)
{
	if (InPathArray.Num() <= 0)
	{
		return;
	}
	FScopedSlowTask SlowTask(InPathArray.Num(), QuickAccessToolLanguage::LoadAssert, true);
	SlowTask.MakeDialog();
	OnMenuClicked(0);
	const int32 Offset = bIsInit ? 0 : QuickAccessArchiveInfo.PathArray.Num();
	for (int32 Index = 0; Index < InPathArray.Num(); Index++)
	{
		const FString& Path = InPathArray[Index];

		SlowTask.EnterProgressFrame(1, FText::Format(QuickAccessToolLanguage::Loading, FText::FromString(Path)));

		if (SlowTask.ShouldCancel())
		{
			break;
		}

		const UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), nullptr, *Path);
		TSharedPtr<SQuickAccessLineWidget> QuickAccessLineWidget;
		SAssignNew(QuickAccessLineWidget, SQuickAccessLineWidget)
		.Path(Path)
		.Index(Index + Offset)
		.Text(FText::FromString(LoadedObj == nullptr ? "None" : LoadedObj->GetName()))
		.IconWidget(GetObjThumbnailByPath(Path))
		.OnItemDrag(this, &SQuickAccessWidget::OnDragItem)
		.OnItemDragStart(this, &SQuickAccessWidget::OnDragItemStart)
		.OnItemDragEnd(this, &SQuickAccessWidget::OnDragItemEnd)
		.OnClicked(this, &SQuickAccessWidget::OnItemClick)
		.OnClearAllClicked(this, &SQuickAccessWidget::OnClearAllFilesClicked)
		.OnSelectAllClicked(this, &SQuickAccessWidget::OnSelectAllClicked);

		QuickAccessLineWidgets.Add(QuickAccessLineWidget);

		QuickAccessLineVertical->AddSlot()
		                       .AutoHeight()
		                       .HAlign(HAlign_Fill)
		[
			QuickAccessLineWidget.ToSharedRef()
		];

		FSlateApplication::Get().PumpMessages();
	}
}

int32 SQuickAccessWidget::GetMenuWidgetIndex() const
{
	return QuickAccessArchiveInfo.ActiveMenuIndex;
}

FReply SQuickAccessWidget::OnMenuClicked(const int32 Index)
{
	if (QuickAccessArchiveInfo.ActiveMenuIndex == Index)
	{
		return FReply::Unhandled();
	}
	if (!MenuButtons.IsValidIndex(Index))
	{
		return FReply::Unhandled();
	}
	MenuButtons[QuickAccessArchiveInfo.ActiveMenuIndex]->SetEnabled(true);
	MenuButtons[Index]->SetEnabled(false);
	QuickAccessArchiveInfo.ActiveMenuIndex = Index;
	QuickAccessArchiveInfo.Save();
	return FReply::Handled();
}

FReply SQuickAccessWidget::OnPickColorClicked()
{
	StartColor = SelectColor;
	FColorPickerArgs ColorPickerArgs;
	ColorPickerArgs.bOnlyRefreshOnMouseUp = true;
	ColorPickerArgs.bIsModal = false;
	ColorPickerArgs.ParentWidget = SharedThis(this);
	ColorPickerArgs.bUseAlpha = false;
	ColorPickerArgs.InitialColorOverride = SelectColor;
	ColorPickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SQuickAccessWidget::OnSelectedStopColorChanged);
	ColorPickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SQuickAccessWidget::OnCancelSelectedStopColorChange);
	OpenColorPicker(ColorPickerArgs);
	return FReply::Handled();
}

void SQuickAccessWidget::OnSelectedStopColorChanged(const FLinearColor InNewColor)
{
	SelectColor = InNewColor;
	if (QuickAccessArchiveInfo.bCopyColorToClipboard)
	{
		const FString ColorString = FString::Printf(
			TEXT("R: %f, G: %f, B: %f, A: %f"), SelectColor.R, SelectColor.G, SelectColor.B, SelectColor.A);
		FPlatformApplicationMisc::ClipboardCopy(*ColorString);
	}
}

void SQuickAccessWidget::OnCancelSelectedStopColorChange(FLinearColor PreviousColor)
{
	SelectColor = StartColor;
}


void SQuickAccessWidget::OnItemClick(const int32 Index)
{
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		LastFirstMenuSelectedIndex = FirstMenuSelectedIndex;
	}
	if (!QuickAccessLineWidgets.IsValidIndex(Index))
	{
		for (auto QuickAccessLineWidget : QuickAccessLineWidgets)
		{
			if (QuickAccessLineWidget.IsValid())
			{
				if (QuickAccessLineWidget->GetIsSelected())
				{
					QuickAccessLineWidget->SetSelected(false);
				}
			}
		}
		FirstMenuSelectedIndex = -1;
		FirstMenuSelectedIndexes.Empty();
		return;
	}
	FirstMenuSelectedIndex = Index;
	const int32 Min = FMath::Min(FirstMenuSelectedIndex, LastFirstMenuSelectedIndex);
	const int32 Max = FMath::Max(FirstMenuSelectedIndex, LastFirstMenuSelectedIndex);
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();
	if (IsShift && !IsControl && !IsAlt)
	{
		for (int32 i = 0; i < QuickAccessLineWidgets.Num(); ++i)
		{
			if (i >= Min && i <= Max)
			{
				FirstMenuSelectedIndexes.AddUnique(i);
			}
		}
	}
	else if (!IsShift && IsControl && !IsAlt)
	{
		FirstMenuSelectedIndexes.AddUnique(Index);
	}
	else
	{
		FirstMenuSelectedIndexes.Empty();
		FirstMenuSelectedIndexes.Add(FirstMenuSelectedIndex);
	}
	FirstMenuSelectedIndexes.Sort([](const int32& A, const int32& B)
	{
		return A > B;
	});

	for (int32 i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetSelected(FirstMenuSelectedIndexes.Contains(i));
	}
}

void SQuickAccessWidget::OnDragItem(const FVector2D Position, const float Offset, const int32 Index)
{
	if (!bIsDragging)
	{
		return;
	}
	if (DragLine.IsValid())
	{
		if (!bIsDragVisible)
		{
			if (FMath::Abs(Offset) > 5)
			{
				bIsDragVisible = true;
				DragLine->SetVisibility(EVisibility::SelfHitTestInvisible);
			}
		}

		const int DragIndex = FMath::Clamp(Position.Y / SQuickAccessLineWidget::GetSize(), -1.f, QuickAccessLineWidgets.Num() - 1.f);
		if (DragIndex == -1)
		{
			DragLine->SetRenderTransform(FSlateRenderTransform(FVector2D(0, DragIndex * SQuickAccessLineWidget::GetSize() + 2)));
		}
		else
		{
			DragLine->SetRenderTransform(FSlateRenderTransform(FVector2D(0, DragIndex * SQuickAccessLineWidget::GetSize() + 2 * DragIndex)));
		}
	}
}

void SQuickAccessWidget::OnDragItemStart(const FVector2D Position, const float Offset, const int32 Index)
{
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();
	if (IsShift || IsControl || IsAlt)
	{
		return;
	}
	bIsDragging = true;
	FirstMenuSelectedIndexes.Empty();
	FirstMenuSelectedIndexes.AddUnique(Index);
	for (int i = QuickAccessLineWidgets.Num() - 1; i >= 0; --i)
	{
		QuickAccessLineWidgets[i]->SetSelected(i == Index);
	}
}

void SQuickAccessWidget::OnDragItemEnd(const FVector2D Position, const float Offset, const int32 Index)
{
	if (!bIsDragging || !bIsDragVisible)
	{
		return;
	}
	bIsDragging = false;
	bIsDragVisible = false;
	DragLine->SetVisibility(EVisibility::Collapsed);
	if (QuickAccessLineWidgets.Num() <= 1)
	{
		return;
	}
	if (!QuickAccessLineWidgets.IsValidIndex(Index) || !QuickAccessArchiveInfo.PathArray.IsValidIndex(Index))
	{
		return;
	}
	const int OffsetInex = Offset / 30;

	const int NewDragIndex = FMath::Clamp(OffsetInex + Index, 0, QuickAccessLineWidgets.Num() - 1);

	if (NewDragIndex == Index)
	{
		return;
	}

	const TSharedPtr<SQuickAccessLineWidget> CurrentWidget = QuickAccessLineWidgets[Index];
	QuickAccessLineWidgets.RemoveAt(Index);
	QuickAccessLineWidgets.Insert(CurrentWidget, NewDragIndex);
	const FString Path = QuickAccessArchiveInfo.PathArray[Index];
	QuickAccessArchiveInfo.PathArray.RemoveAt(Index);
	QuickAccessArchiveInfo.PathArray.Insert(Path, NewDragIndex);

	QuickAccessLineVertical->ClearChildren();
	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);

		QuickAccessLineVertical->AddSlot()
		                       .AutoHeight()
		                       .HAlign(HAlign_Fill)
		[
			QuickAccessLineWidgets[i].ToSharedRef()
		];
	}
	QuickAccessArchiveInfo.Save();
}

TSharedRef<SWidget> SQuickAccessWidget::GetObjThumbnailByPath(const FString& Path)
{
	const FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(FName(*Path));
	const TSharedPtr<FAssetThumbnail> AssetThumbnail = MakeShareable(new FAssetThumbnail(AssetData, SQuickAccessLineWidget::GetIconSize(), SQuickAccessLineWidget::GetIconSize(), nullptr));
	return AssetThumbnail->MakeThumbnailWidget();
}

void SQuickAccessWidget::OnBrowseAssetClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickAccessLineWidgets[FirstMenuSelectedIndex]->BrowserToObject();
	}
}

void SQuickAccessWidget::OnReferenceViewerClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
	{
		QuickAccessLineWidgets[FirstMenuSelectedIndex]->ReferenceViewer();
	}
}

void SQuickAccessWidget::OnExploreFolderClicked()
{
	if (FirstMenuSelectedIndex < 0)
	{
		return;
	}
	for (const int32 Index : FirstMenuSelectedIndexes)
	{
		if (QuickAccessLineWidgets.IsValidIndex(Index))
		{
			QuickAccessLineWidgets[Index]->ExploreFolder();
		}
	}
}

FReply SQuickAccessWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bShift = InKeyEvent.IsShiftDown();
	const bool bCtrl = InKeyEvent.IsControlDown();
	const bool bAlt = InKeyEvent.IsAltDown();

	const FInputChord CurrentInputChord(Key, bShift, bCtrl, bAlt, false);

	for (auto ChordFunction : ChordFunctionMap)
	{
		if (ChordFunction.Key == CurrentInputChord)
		{
			ChordFunction.Value();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SQuickAccessWidget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SQuickAccessWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnItemClick(-1);
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SQuickAccessWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && !IsShift && !IsControl && !IsAlt)
	{
		const TSharedPtr<SWidget> MenuWidget = CreateRightClickMenu();
		if (MenuWidget.IsValid())
		{
			const FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr
				                               ? *MouseEvent.GetEventPath()
				                               : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(),
			                                  WidgetPath,
			                                  MenuWidget.ToSharedRef(),
			                                  MouseEvent.GetScreenSpacePosition(),
			                                  FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu
			                                  )
			);
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SQuickAccessWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
	if (!MenuTexts.IsValidIndex(QuickAccessArchiveInfo.ActiveMenuIndex) || FMath::IsNearlyEqual(MouseEvent.GetWheelDelta(), 0))
	{
		return Reply;
	}

	if (MenuTexts[QuickAccessArchiveInfo.ActiveMenuIndex].ToString() == QuickAccessToolLanguage::CustomTask.ToString())
	{
		QuickAccessArchiveInfo.CustomTaskFontSize = FMath::Clamp(static_cast<int32>(QuickAccessArchiveInfo.CustomTaskFontSize + MouseEvent.GetWheelDelta()), 5, 30);
		CustomTaskMultiLineEditableText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", QuickAccessArchiveInfo.CustomTaskFontSize));
		QuickAccessArchiveInfo.Save();
		return FReply::Handled();
	}
	return Reply;
}

FReply SQuickAccessWidget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	const bool IsShift = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	const bool IsControl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	const bool IsAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();
	if (!IsShift && !IsControl && !IsAlt)
	{
		return OnAddObjectClicked.Execute();
	}

	return SCompoundWidget::OnDrop(MyGeometry, DragDropEvent);
}

TSharedPtr<SWidget> SQuickAccessWidget::CreateRightClickMenu()
{
	if (!MenuTexts.IsValidIndex(QuickAccessArchiveInfo.ActiveMenuIndex))
	{
		return nullptr;
	}
	FMenuBuilder MenuBuilder(true, nullptr);
	if (MenuTexts[QuickAccessArchiveInfo.ActiveMenuIndex].ToString() == QuickAccessToolLanguage::QuickPanel.ToString())
	{
		CreateQuickPanelMenu(MenuBuilder);
	}

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::ClipboardTexture, FText::FromString("    (Alt + V)")),
		QuickAccessToolLanguage::ClipboardSaveAsTextureToolTips,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			UQuickAccessLibrary::SaveClipboardToAsset();
		}))
	);
	return MenuBuilder.MakeWidget();
}

void SQuickAccessWidget::CreateQuickPanelMenu(FMenuBuilder& MenuBuilder)
{
	TWeakPtr<SQuickAccessWidget> TempThis = SharedThis(this);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::SelectAllFilesFormat, FText::FromString("    (Ctrl + A)")),
		QuickAccessToolLanguage::SelectAllFilesTooltip,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnSelectAllClicked();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::ClearAllFilesFormat, FText::FromString("    (Ctrl + Delete)")),
		QuickAccessToolLanguage::ClearAllFilesTooltip,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnClearAllFilesClicked();
			}
		}))
	);
	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::SaveAllFiles, FText::FromString("    (Ctrl + Shift + S)")),
		QuickAccessToolLanguage::SaveAllFilesTipsToolTips,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnSaveAllClicked();
			}
		}))
	);
}

void SQuickAccessWidget::OnClearAllFilesClicked()
{
	QuickAccessLineWidgets.Empty();
	QuickAccessLineVertical->ClearChildren();
	FirstMenuSelectedIndex = -1;
	LastFirstMenuSelectedIndex = -1;
	QuickAccessArchiveInfo.PathArray.Empty();
	FirstMenuSelectedIndexes.Empty();
	QuickAccessArchiveInfo.Save();
}

void SQuickAccessWidget::OnSelectAllClicked()
{
	FirstMenuSelectedIndexes.Empty();

	for (int i = QuickAccessLineWidgets.Num() - 1; i >= 0; --i)
	{
		QuickAccessLineWidgets[i]->SetSelected(true);
		FirstMenuSelectedIndexes.Add(i);
	}
}

void SQuickAccessWidget::OnSaveClicked()
{
	for (const int32& TempSelectedIndex : FirstMenuSelectedIndexes)
	{
		if (QuickAccessLineWidgets.IsValidIndex(FirstMenuSelectedIndex))
		{
			QuickAccessLineWidgets[TempSelectedIndex]->Save();
		}
	}
}

void SQuickAccessWidget::OnSaveAllClicked()
{
	for (TSharedPtr<SQuickAccessLineWidget> QuickAccessLineWidget : QuickAccessLineWidgets)
	{
		if (QuickAccessLineWidget.IsValid())
		{
			QuickAccessLineWidget->Save();
		}
	}
}

void SQuickAccessWidget::OnDeleteObject()
{
	if (FirstMenuSelectedIndexes.Num() <= 0)
	{
		return;
	}
	for (int i = 0; i < FirstMenuSelectedIndexes.Num(); ++i)
	{
		const int Index = FirstMenuSelectedIndexes[i];
		if (QuickAccessLineWidgets.IsValidIndex(Index))
		{
			QuickAccessArchiveInfo.PathArray.RemoveAt(Index);
			QuickAccessLineVertical->RemoveSlot(QuickAccessLineWidgets[Index].ToSharedRef());
			QuickAccessLineWidgets.RemoveAt(Index);
		}
	}
	for (int i = 0; i < QuickAccessLineWidgets.Num(); ++i)
	{
		QuickAccessLineWidgets[i]->SetIndex(i);
	}

	FirstMenuSelectedIndexes.Empty();
	QuickAccessArchiveInfo.Save();
}

void SQuickAccessWidget::OnAddObjects(TArray<FString> NewPath)
{
	TArray<FString> Path;
	for (int i = 0; i < NewPath.Num(); ++i)
	{
		if (!QuickAccessArchiveInfo.PathArray.Contains(NewPath[i]))
		{
			Path.AddUnique(NewPath[i]);
		}
	}
	AddChildren(Path);
	QuickAccessArchiveInfo.PathArray.Append(Path);
	QuickAccessArchiveInfo.Save();
}

const TArray<FString>& SQuickAccessWidget::GetPathArray()
{
	return QuickAccessArchiveInfo.PathArray;
}

bool SQuickAccessWidget::SupportsKeyboardFocus() const
{
	return true;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
