// Fill out your copyright notice in the Description page of Project Settings.

#include "ToolWidget.h"

#include "Page/CustomCommandsPanel.h"
#include "Page/QuickCommonWidget.h"
#include "Page/QuickPanel.h"
#include "Page/QuickTaskWidget.h"
#include "QuickAccessTool/Common/QuickAccessLibrary.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "QuickAccessTool/Language/Language.h"
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

void SToolWidget::Construct(const FArguments& InArgs)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.Load();

	OnAddObjectClicked = InArgs._OnAddObjectClicked;

	TWeakPtr<SToolWidget> TempThis = SharedThis(this);
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

	TitleButtonStyle = MakeUnique<FButtonStyle>();
	TitleButtonStyle->SetNormal(FSlateNoResource());
	TitleButtonStyle->SetHovered(FSlateColorBrush(FLinearColor(1, 0.4, 0, 0.65)));
	TitleButtonStyle->SetPressed(FSlateNoResource());
	TitleButtonStyle->SetNormalPadding(FMargin(2, 2, 2, 2));
	TitleButtonStyle->SetPressedPadding(FMargin(2, 2, 2, 2));
	TitleBlockStyle = MakeUnique<FTextBlockStyle>();
	TitleBlockStyle->ColorAndOpacity = FLinearColor::White;


	SAssignNew(MenuHorizontalBox, SHorizontalBox);
	for (int32 i = 0; i < MenuTexts.Num(); i++)
	{
		TSharedPtr<SButton> MenuButton;
		SAssignNew(MenuButton, SButton)
		.ButtonStyle(TitleButtonStyle.Get())
		.Text(MenuTexts[i])
		.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
		.OnClicked(this, &SToolWidget::OnMenuClicked, i);

		MenuHorizontalBox->AddSlot()
		                 .HAlign(HAlign_Right)
		                 .AutoWidth()
		[
			MenuButton.ToSharedRef()
		];
		MenuButtons.Add(MenuButton);
		if (FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex == i)
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
					.WidgetIndex(this, &SToolWidget::GetMenuWidgetIndex)
					+ SWidgetSwitcher::Slot()
					[
						SAssignNew(QuickPanel, SQuickPanel)
					]
					+ SWidgetSwitcher::Slot()
					.Padding(4.0f, 0, 0, 0)
					[
						SAssignNew(QuickCommonWidget, SQuickCommonWidget)
					]
					+ SWidgetSwitcher::Slot()
					.Padding(4.0f, 0, 0, 0)
					[
						SAssignNew(QuickTaskWidget, SQuickTaskWidget)
					]
					+ SWidgetSwitcher::Slot()
					.Padding(4.0f, 0, 0, 0)
					[
						SAssignNew(CustomCommandsPanel, SCustomCommandsPanel)
					]
				]
			]
		]
	];

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");

	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &SToolWidget::OnAssetRemoved);
}

void SToolWidget::OnAssetRemoved(const FAssetData& AssetData) const
{
	if (QuickPanel.IsValid())
	{
		if (QuickPanel->OnAssetRemoved(AssetData))
		{
			FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
		}
	}
}

int32 SToolWidget::GetMenuWidgetIndex() const
{
	return FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex;
}

FReply SToolWidget::OnMenuClicked(const int32 Index)
{
	if (FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex == Index)
	{
		return FReply::Unhandled();
	}
	if (!MenuButtons.IsValidIndex(Index))
	{
		return FReply::Unhandled();
	}
	MenuButtons[FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex]->SetEnabled(true);
	MenuButtons[Index]->SetEnabled(false);
	FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex = Index;
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	return FReply::Handled();
}


void SToolWidget::OnBrowseAssetClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnBrowseAssetClicked();
	}
}

void SToolWidget::OnReferenceViewerClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnReferenceViewerClicked();
	}
}

void SToolWidget::OnExploreFolderClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnExploreFolderClicked();
	}
}

FReply SToolWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
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

FReply SToolWidget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SToolWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (QuickPanel.IsValid())
		{
			QuickPanel->OnItemClick(-1);
		}
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SToolWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
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

FReply SToolWidget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
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

FReply SToolWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (QuickTaskWidget.IsValid())
	{
		QuickTaskWidget->OnMouseWheel(MyGeometry, MouseEvent);
	}
	return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

TSharedPtr<SWidget> SToolWidget::CreateRightClickMenu()
{
	if (!MenuTexts.IsValidIndex(FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex))
	{
		return nullptr;
	}
	FMenuBuilder MenuBuilder(true, nullptr);
	if (MenuTexts[FQuickAccessToolModule::QuickAccessArchiveInfo.ActiveMenuIndex].ToString() == QuickAccessToolLanguage::QuickPanel.ToString())
	{
		CreateQuickPanelMenu(MenuBuilder);
		MenuBuilder.AddMenuSeparator();
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

void SToolWidget::CreateQuickPanelMenu(FMenuBuilder& MenuBuilder)
{
	TWeakPtr<SToolWidget> TempThis = SharedThis(this);

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

void SToolWidget::OnClearAllFilesClicked()
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnClearAllFilesClicked();
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Empty();
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SToolWidget::OnSelectAllClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnSelectAllClicked();
	}
}

void SToolWidget::OnSaveClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnSaveClicked();
	}
}

void SToolWidget::OnSaveAllClicked() const
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnSaveAllClicked();
	}
}

void SToolWidget::OnDeleteObject()
{
	if (QuickPanel.IsValid())
	{
		QuickPanel->OnDeleteObject();
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SToolWidget::EventOnKeyDown(const FKey& InKey) const
{
	if (CustomCommandsPanel.IsValid())
	{
		CustomCommandsPanel->EventOnKeyDown(InKey);
	}
}

void SToolWidget::OnAddObjects(TArray<FString> NewPath)
{
	TArray<FString> Path;
	for (int i = 0; i < NewPath.Num(); ++i)
	{
		if (!FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Contains(NewPath[i]))
		{
			Path.AddUnique(NewPath[i]);
		}
	}
	if (NewPath.Num() > 0)
	{
		OnMenuClicked(0);
		if (QuickPanel.IsValid())
		{
			QuickPanel->OnAddObjects(Path);
		}
		FQuickAccessToolModule::QuickAccessArchiveInfo.PathArray.Append(Path);
		FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	}
}

bool SToolWidget::SupportsKeyboardFocus() const
{
	return true;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
