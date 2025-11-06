// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickAccessLineWidget.h"

#include "FileHelpers.h"
#if ENGINE_MAJOR_VERSION == 4
#elif ENGINE_MAJOR_VERSION == 5
#include "EditorStyleSet.h"
#include "UObject/SavePackage.h"
#endif
#include "ReferenceViewer/EdGraph_ReferenceViewer.h"
#include "QuickAccessButton.h"
#include "SlateOptMacros.h"
#include "QuickAccessTool/Common/QuickAccessLibrary.h"
#include "QuickAccessTool/Language/Language.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickAccessLineWidget::Construct(const FArguments& InArgs)
{
	IsSelected = InArgs._IsSelected.Get();
	Text = InArgs._Text.Get();

	OnClicked = InArgs._OnClicked;
	OnItemDrag = InArgs._OnItemDrag;
	OnItemDragStart = InArgs._OnItemDragStart;
	OnItemDragEnd = InArgs._OnItemDragEnd;
	OnSelectAllClicked = InArgs._OnSelectAllClicked;
	OnClearAllClicked = InArgs._OnClearAllClicked;

	Path = InArgs._Path.Get();
	IconWidget = InArgs._IconWidget.Get();
	Index = InArgs._Index.Get();
	if (!IconWidget.IsValid())
	{
		check(false);
		return;
	}
	SAssignNew(Button, SQuickAccessButton)
	.OnDoubleClicked(this, &SQuickAccessLineWidget::OnButtonDoubleClick)
	.ButtonStyle(&WidgetStyle)
	.OnPressed(this, &SQuickAccessLineWidget::OnButtonPressed)
	.OnReleased(this, &SQuickAccessLineWidget::OnButtonReleased)
	.OnClicked(this, &SQuickAccessLineWidget::OnButtonClick);

	SAssignNew(TextBlock, STextBlock)
	.Visibility(EVisibility::SelfHitTestInvisible)
	.Text(Text);

	AssetDirtyBrush = MakeUnique<FSlateBrush>(*FEditorStyle::GetBrush("ContentBrowser.ContentDirty"));
	AssetDirtyBrush->ImageSize = FVector2D(10, 10);
	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(GetSize())
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				Button.ToSharedRef()
			]
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(GetOffset(), 0, GetOffset(), 0))
				[
					SNew(SBox)
					.WidthOverride(GetIconSize())
					.HeightOverride(GetIconSize())
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							IconWidget.ToSharedRef()
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(this, &SQuickAccessLineWidget::GetDirtyImage)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					TextBlock.ToSharedRef()
				]
			]
		]
	];

	RefreshButtonState();
}

void SQuickAccessLineWidget::Tick(const FGeometry& AllottedGeometry, const double CurrentTime, float DeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, CurrentTime, DeltaTime);

	if (bIsDragging)
	{
		const FVector2D CurrentMousePosition = FSlateApplication::Get().GetCursorPos();
		const FVector2D Delta = CurrentMousePosition - InitialMousePosition;
		DragPosition = FVector2D(AllottedGeometry.Position.X, AllottedGeometry.Position.Y) + DragOffset;
		DragOffset = Delta;
		OnItemDrag.ExecuteIfBound(DragPosition, DragOffset.Y, GetIndex());
	}
}

void SQuickAccessLineWidget::OnButtonPressed()
{
	bIsDragging = true;
	InitialMousePosition = FSlateApplication::Get().GetCursorPos();
	DragPosition = FVector2D(GetCachedGeometry().Position.X, GetCachedGeometry().Position.Y);
	OnItemDragStart.ExecuteIfBound(DragPosition, DragOffset.Y, GetIndex());
}

void SQuickAccessLineWidget::OnButtonReleased()
{
	if (bIsDragging)
	{
		bIsDragging = false;
		OnItemDragEnd.ExecuteIfBound(DragPosition, DragOffset.Y, GetIndex());
		DragOffset = FVector2D(0, 0);
	}
}

FReply SQuickAccessLineWidget::OnButtonClick() const
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute(GetIndex());
	}
	return FReply::Unhandled();
}

FReply SQuickAccessLineWidget::OnButtonDoubleClick() const
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(GetObject());
	return FReply::Handled();
}

void SQuickAccessLineWidget::SetSelected(const bool bIsSelected)
{
	if (bIsSelected == IsSelected)
	{
		return;
	}
	IsSelected = bIsSelected;
	RefreshButtonState();
}

void SQuickAccessLineWidget::RefreshButtonState()
{
	if (IsSelected)
	{
		WidgetStyle.Normal.TintColor = PressedTintColor;
		WidgetStyle.Hovered.TintColor = PressedTintColor;
		WidgetStyle.Pressed.TintColor = PressedTintColor;
		TextBlock->SetColorAndOpacity(FLinearColor::Black);
	}
	else
	{
		WidgetStyle.Normal.TintColor = NormalTintColor;
		WidgetStyle.Hovered.TintColor = HoveredTintColor;
		WidgetStyle.Pressed.TintColor = PressedTintColor;
		TextBlock->SetColorAndOpacity(FLinearColor::White);
	}
}

bool SQuickAccessLineWidget::GetIsSelected() const
{
	return IsSelected;
}

void SQuickAccessLineWidget::BrowserToObject() const
{
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(GetObject());
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}

void SQuickAccessLineWidget::ReferenceViewer() const
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	AssetIdentifiers.Add(FAssetIdentifier(GetObject()->GetOutermost()->GetFName()));
	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
}

void SQuickAccessLineWidget::ExploreFolder() const
{
	if (IsValid(GetObject()))
	{
		const FString PackageName = GetObject()->GetOutermost()->GetName();
		FString Filename;

		if (FPackageName::DoesPackageExist(PackageName, nullptr, &Filename))
		{
			FPlatformProcess::ExploreFolder(*Filename);
		}
	}
}

void SQuickAccessLineWidget::Save() const
{
	const UObject* Object = GetObject();
	if (!Object)
	{
		return;
	}
	UPackage* Package = Object->GetPackage();
	if (!Package)
	{
		return;
	}

	if (Package->IsDirty())
	{
		FEditorFileUtils::PromptForCheckoutAndSave({Package}, false, false);
	}
}

FReply SQuickAccessLineWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SQuickAccessLineWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		const TSharedPtr<SWidget> MenuWidget = CreateRightClickMenu();
		if (MenuWidget.IsValid())
		{
			const FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr
				                               ? *MouseEvent.GetEventPath()
				                               : FWidgetPath();
			FSlateApplication::Get().PushMenu(
				AsShared(),
				WidgetPath,
				MenuWidget.ToSharedRef(),
				MouseEvent.GetScreenSpacePosition(),
				FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
			);
		}

		return FReply::Handled();
	}

	return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
}

TSharedPtr<SWidget> SQuickAccessLineWidget::CreateRightClickMenu()
{
	if (!IsValid(GetObject()))
	{
		return SNullWidget::NullWidget;
	}
	FMenuBuilder MenuBuilder(true, nullptr);

	TWeakPtr<SQuickAccessLineWidget> TempThis = SharedThis(this);
	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::SelectAllFilesFormat, FText::FromString("    (Ctrl + A)")),
		QuickAccessToolLanguage::SelectAllFilesTooltip,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnSelectAllClicked.ExecuteIfBound();
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
				TempThis.Pin()->OnClearAllClicked.Execute();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::BrowseToAssetFormat, FText::FromString("    (Ctrl + B)")),
		FText::Format(QuickAccessToolLanguage::BrowseToTooltipFormat, FText::FromString(Path)),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->BrowserToObject();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::OpenFileFormat, FText::FromString("    (Double Click)")),
		FText::Format(QuickAccessToolLanguage::OpenFileTooltipFormat, FText::FromString(Path)),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				if (IsValid(TempThis.Pin()->GetObject()))
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(
						TempThis.Pin()->GetObject());
				}
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::ReferenceViewFormat, FText::FromString("    (Alt + Shift + R)")),
		FText::Format(QuickAccessToolLanguage::ReferenceViewTooltipFormat,
		              FText::FromString(GetObject()->GetPathName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->ReferenceViewer();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		QuickAccessToolLanguage::CopyFileName,
		FText::Format(QuickAccessToolLanguage::CopyFileNameTooltipFormat, FText::FromString(GetObject()->GetName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				if (IsValid(TempThis.Pin()->GetObject()))
				{
					FPlatformApplicationMisc::ClipboardCopy(*TempThis.Pin()->GetObject()->GetName());
				}
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		QuickAccessToolLanguage::CopyFileReference,
		FText::Format(QuickAccessToolLanguage::CopyFileReferenceTooltipFormat,
		              FText::FromString(GetObject()->GetFullName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				if (IsValid(TempThis.Pin()->GetObject()))
				{
					FPlatformApplicationMisc::ClipboardCopy(*TempThis.Pin()->GetObject()->GetFullName());
				}
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		QuickAccessToolLanguage::CopyFilePath,
		FText::Format(QuickAccessToolLanguage::CopyFilePathTooltipFormat,
		              FText::FromString(GetObject()->GetPathName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				if (IsValid(TempThis.Pin()->GetObject()))
				{
					FPlatformApplicationMisc::ClipboardCopy(*TempThis.Pin()->GetObject()->GetPathName());
				}
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::ShowInExplorerFormat, FText::FromString("    (Ctrl + Shift + Q)")),
		FText::Format(QuickAccessToolLanguage::ShowInExplorerTooltipFormat,
		              FText::FromString(GetObject()->GetPathName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->ExploreFolder();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		FText::Format(QuickAccessToolLanguage::SaveFile, FText::FromString("    (Ctrl + S)")),
		FText::Format(QuickAccessToolLanguage::SaveFileToolTips,
		              FText::FromString(GetObject()->GetPathName())),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->Save();
			}
		}))
	);

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

void SQuickAccessLineWidget::SetIndex(const int32 InIndex)
{
	Index = InIndex;
}

int32 SQuickAccessLineWidget::GetIndex() const
{
	return Index;
}

UObject* SQuickAccessLineWidget::GetObject() const
{
	return StaticLoadObject(UObject::StaticClass(), nullptr, *Path);
}

const FSlateBrush* SQuickAccessLineWidget::GetDirtyImage() const
{
	const UObject* Object = GetObject();
	if (!Object)
	{
		return nullptr;
	}
	const UPackage* Package = Object->GetPackage();
	if (!Package)
	{
		return nullptr;
	}
	return Package->IsDirty() ? AssetDirtyBrush.Get() : nullptr;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
