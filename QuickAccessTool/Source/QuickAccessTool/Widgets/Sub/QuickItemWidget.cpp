// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickItemWidget.h"

#include "FileHelpers.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"
#if ENGINE_MAJOR_VERSION == 4
#elif ENGINE_MAJOR_VERSION == 5
#include "EditorStyleSet.h"
#include "UObject/SavePackage.h"
#endif
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "ReferenceViewer/EdGraph_ReferenceViewer.h"
#include "SlateOptMacros.h"
#include "QuickAccessTool/Common/QuickAccessLibrary.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Widgets/Base/DoubleClickButton.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQuickItemWidget::Construct(const FArguments& InArgs)
{
	IsSelected = InArgs._IsSelected.Get();
	Text = InArgs._Text.Get();
	TabName = InArgs._TabName.Get();	

	OnClicked = InArgs._OnClicked;
	OnItemDrag = InArgs._OnItemDrag;
	OnItemDragStart = InArgs._OnItemDragStart;
	OnItemDragEnd = InArgs._OnItemDragEnd;
	OnSelectAllClicked = InArgs._OnSelectAllClicked;
	OnClearAllClicked = InArgs._OnClearAllClicked;

	OnAddNewTabClicked = InArgs._OnAddNewTabClicked;
	OnRemoveTabClicked = InArgs._OnRemoveTabClicked;
	OnMoveToClick = InArgs._OnMoveToClick;
	Path = InArgs._Path.Get();
	IconWidget = InArgs._IconWidget.Get();
	Index = InArgs._Index.Get();
	if (!IconWidget.IsValid())
	{
		check(false);
		return;
	}
	SAssignNew(Button, SDoubleClickButton)
	.OnDoubleClicked(this, &SQuickItemWidget::OnButtonDoubleClick)
	.ButtonStyle(&WidgetStyle)
	.OnPressed(this, &SQuickItemWidget::OnButtonPressed)
	.OnReleased(this, &SQuickItemWidget::OnButtonReleased)
	.OnClicked(this, &SQuickItemWidget::OnButtonClick);

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
							.Image(this, &SQuickItemWidget::GetDirtyImage)
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

void SQuickItemWidget::Tick(const FGeometry& AllottedGeometry, const double CurrentTime, float DeltaTime)
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

void SQuickItemWidget::OnButtonPressed()
{
	bIsDragging = true;
	InitialMousePosition = FSlateApplication::Get().GetCursorPos();
	DragPosition = FVector2D(GetCachedGeometry().Position.X, GetCachedGeometry().Position.Y);
	OnItemDragStart.ExecuteIfBound(DragPosition, DragOffset.Y, GetIndex());
}

void SQuickItemWidget::OnButtonReleased()
{
	if (bIsDragging)
	{
		bIsDragging = false;
		OnItemDragEnd.ExecuteIfBound(DragPosition, DragOffset.Y, GetIndex());
		DragOffset = FVector2D(0, 0);
	}
}

FReply SQuickItemWidget::OnButtonClick() const
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute(GetIndex());
	}
	return FReply::Unhandled();
}

FReply SQuickItemWidget::OnButtonDoubleClick() const
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(GetObject());
	return FReply::Handled();
}

void SQuickItemWidget::SetSelected(const bool bIsSelected)
{
	if (bIsSelected == IsSelected)
	{
		return;
	}
	IsSelected = bIsSelected;
	RefreshButtonState();
}

void SQuickItemWidget::RefreshButtonState()
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

bool SQuickItemWidget::GetIsSelected() const
{
	return IsSelected;
}

void SQuickItemWidget::BrowserToObject() const
{
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(GetObject());
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}

void SQuickItemWidget::ReferenceViewer() const
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	AssetIdentifiers.Add(FAssetIdentifier(GetObject()->GetOutermost()->GetFName()));
	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
}

void SQuickItemWidget::ExploreFolder() const
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

void SQuickItemWidget::RunEditorUtilityWidget() const
{
	UObject* Object = GetObject();
	if (!Object)
	{
		return;
	}

	if (UEditorUtilityWidgetBlueprint* WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(Object))
	{
		if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
		{
			FName TabID;
			EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(WidgetBlueprint, TabID);
		}
	}
}

void SQuickItemWidget::Save() const
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

FReply SQuickItemWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SQuickItemWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
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

TSharedPtr<SWidget> SQuickItemWidget::CreateRightClickMenu()
{
	if (!IsValid(GetObject()))
	{
		return SNullWidget::NullWidget;
	}
	FMenuBuilder MenuBuilder(true, nullptr);

	TWeakPtr<SQuickItemWidget> TempThis = SharedThis(this);
	UObject* Object = GetObject();

	MenuBuilder.AddSubMenu(
		QuickAccessToolLanguage::MoveToNewPage,
		QuickAccessToolLanguage::MoveToAnotherPage,
		FNewMenuDelegate::CreateLambda([TempThis](FMenuBuilder& SubMenuBuilder)
		{
			if (!TempThis.IsValid()) return;

			TArray<FString> AllTabNames;
			FQuickAccessToolModule::QuickAccessArchiveInfo.MultiPathMap.GetKeys(AllTabNames);
			for (const FString& TabName : AllTabNames)
			{
				if (TempThis.Pin()->TabName == TabName)
				{
					continue;
				}
				SubMenuBuilder.AddMenuEntry(
					FText::FromString(TabName),
					FText::Format(QuickAccessToolLanguage::MoveToAnotherPage, FText::FromString(TabName)),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([TempThis, TabName]()
					{
						TempThis.Pin()->OnMoveToClick.ExecuteIfBound(TabName, TempThis.Pin()->GetIndex());
					}))
				);
			}
		})
	);

	MenuBuilder.AddMenuEntry(
		QuickAccessToolLanguage::AddNewTab,
		QuickAccessToolLanguage::AddNewTabTooltip,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnAddNewTabClicked.ExecuteIfBound();
			}
		}))
	);

	MenuBuilder.AddMenuEntry(
		QuickAccessToolLanguage::RemoveCurrentTab,
		QuickAccessToolLanguage::RemoveCurrentTabTooltip,
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([TempThis]()
		{
			if (TempThis.IsValid())
			{
				TempThis.Pin()->OnRemoveTabClicked.ExecuteIfBound();
			}
		}))
	);
	MenuBuilder.AddMenuSeparator();
	if (Object)
	{
		if (UEditorUtilityWidgetBlueprint* WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(Object))
		{
			MenuBuilder.AddMenuEntry(QuickAccessToolLanguage::RunEditorUtilityWidget,
			                         QuickAccessToolLanguage::RunEditorUtilityWidget,
			                         FSlateIcon(),
			                         FUIAction(FExecuteAction::CreateLambda([TempThis]()
			                         {
				                         if (TempThis.IsValid())
				                         {
					                         TempThis.Pin()->RunEditorUtilityWidget();
				                         }
			                         }))
			);
			MenuBuilder.AddMenuSeparator();
		}
	}

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

	MenuBuilder.AddMenuSeparator();

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

	MenuBuilder.AddMenuSeparator();

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

	MenuBuilder.AddMenuSeparator();

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

void SQuickItemWidget::SetIndex(const int32 InIndex)
{
	Index = InIndex;
}

int32 SQuickItemWidget::GetIndex() const
{
	return Index;
}

UObject* SQuickItemWidget::GetObject() const
{
	return StaticLoadObject(UObject::StaticClass(), nullptr, *Path);
}

const FSlateBrush* SQuickItemWidget::GetDirtyImage() const
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

void SQuickItemWidget::RenameTab(const FString& OldName, const FString& NewName)
{
	TabName = NewName;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
