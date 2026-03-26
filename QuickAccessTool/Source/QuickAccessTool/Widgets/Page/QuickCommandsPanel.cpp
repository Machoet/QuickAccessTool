#include "QuickCommandsPanel.h"
#include "SKeySelector.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/STableRow.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"

void SQuickCommandsPanel::Construct(const FArguments& InArgs)
{
	RefreshDisplayItems();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(QuickAccessToolLanguage::CustomCommandPanel)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.Justification(ETextJustify::Center)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5, 10, 5, 5)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.Padding(0, 0, 5, 0)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(QuickAccessToolLanguage::CommandLabel)
					.MinDesiredWidth(50)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(NewCommandInput, SEditableTextBox)
					.HintText(QuickAccessToolLanguage::CommandHint)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.Padding(0, 0, 5, 0)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(QuickAccessToolLanguage::DescLabel)
					.MinDesiredWidth(50)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(NewDescriptionInput, SEditableTextBox)
					.HintText(QuickAccessToolLanguage::DescHint)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.Padding(0, 0, 5, 0)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(QuickAccessToolLanguage::Shortcuts)
					.MinDesiredWidth(50)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SKeySelector)
					.CurrentKey_Lambda([this]() -> TOptional<FKey> { return CurrentKey; })
					.OnKeyChanged_Lambda([this](const TSharedPtr<FKey>& SelectedKey) { CurrentKey = *SelectedKey.Get(); })
					.FilterBlueprintBindable(false)
					.AllowClear(true)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::AddButton)
					.OnClicked(this, &SQuickCommandsPanel::OnAddButtonClicked)
					.HAlign(HAlign_Center)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::DeleteButton)
					.OnClicked(this, &SQuickCommandsPanel::OnDeleteButtonClicked)
					.HAlign(HAlign_Center)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::ClearButton)
					.OnClicked_Lambda([this]() -> FReply
					{
						ClearAllCommands();
						return FReply::Handled();
					})
					.HAlign(HAlign_Center)
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SAssignNew(CommandListView, SListView<TSharedPtr<FCommandItem>>)
				                                                                .ListItemsSource(&CommandItems)
				                                                                .OnGenerateRow(this, &SQuickCommandsPanel::OnGenerateCommandRow)
				                                                                .OnMouseButtonDoubleClick(this, &SQuickCommandsPanel::OnCommandItemDoubleClicked) // 双击编辑
				                                                                .SelectionMode(ESelectionMode::Single)
			]
		]
	];
}

TSharedRef<ITableRow> SQuickCommandsPanel::OnGenerateCommandRow(
	TSharedPtr<FCommandItem> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FCommandItem>>, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item->DescriptionText))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ToolTipText(FText::FromString(Item->CommandText))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SKeySelector)
					.CurrentKey_Lambda([this, Item]() -> TOptional<FKey> { return GetCurrentKey(Item); })
					.OnKeyChanged_Lambda([this, Item](const TSharedPtr<FKey>& SelectedKey) { OnKeyChanged(SelectedKey, Item); })
					.FilterBlueprintBindable(false)
					.AllowClear(true)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::ExecuteButton)
					.OnClicked_Lambda([this, Item]() -> FReply
					{
						OnCommandItemClicked(Item);
						return FReply::Handled();
					})
					.ToolTipText(QuickAccessToolLanguage::ExecuteTooltip)
				]
			]
		];
}

void SQuickCommandsPanel::OnCommandItemDoubleClicked(TSharedPtr<FCommandItem> Item)
{
	if (!Item.IsValid()) return;

	SelectedItem = Item;

	NewDescriptionInput->SetText(FText::FromString(Item->DescriptionText == Item->CommandText ? "" : Item->DescriptionText));
	NewCommandInput->SetText(FText::FromString(Item->CommandText));
	CurrentKey = Item->BindKey;
}

void SQuickCommandsPanel::OnCommandItemClicked(TSharedPtr<FCommandItem> Item)
{
	if (!Item.IsValid()) return;
	ExecuteCommand(Item->CommandText);
}

void SQuickCommandsPanel::ExecuteCommand(const FString& Command)
{
#if WITH_EDITOR
	if (GEditor)
	{
		if (GEditor->PlayWorld)
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(GEditor->PlayWorld, Command, nullptr);
			return;
		}
		else
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(GEditor->GetEditorWorldContext().World(), Command, nullptr);
			return;
		}
	}
#endif

	if (const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(World, Command, nullptr);
	}
}

FReply SQuickCommandsPanel::OnAddButtonClicked()
{
	FString Description = NewDescriptionInput->GetText().ToString();
	FString Command = NewCommandInput->GetText().ToString();

	if (Command.IsEmpty())
	{
		return FReply::Handled();
	}

	if (Description.IsEmpty())
	{
		Description = Command;
	}

	TSharedPtr<FCommandItem> TempSelectedItem = nullptr;
	if (CommandListView.IsValid())
	{
		TArray<TSharedPtr<FCommandItem>> SelectedItems = CommandListView->GetSelectedItems();
		if (SelectedItems.Num() > 0)
		{
			TempSelectedItem = SelectedItems[0];
		}
	}

	if (TempSelectedItem.IsValid())
	{
		if (TempSelectedItem->DescriptionText != Description)
		{
			FQuickAccessToolModule::QuickAccessArchiveInfo.AddCommand(Description, Command, CurrentKey);
		}
		NewCommandInput->SetText(FText::GetEmpty());
		NewDescriptionInput->SetText(FText::GetEmpty());
		CurrentKey = EKeys::Invalid;
	}
	else
	{
		FQuickAccessToolModule::QuickAccessArchiveInfo.AddCommand(Description, Command, CurrentKey);
		NewCommandInput->SetText(FText::GetEmpty());
		NewDescriptionInput->SetText(FText::GetEmpty());
		CurrentKey = EKeys::Invalid;
	}

	RefreshDisplayItems();

	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();

	return FReply::Handled();
}

FReply SQuickCommandsPanel::OnDeleteButtonClicked()
{
	if (!CommandListView.IsValid()) return FReply::Handled();

	TArray<TSharedPtr<FCommandItem>> SelectedItems = CommandListView->GetSelectedItems();
	if (SelectedItems.Num() > 0)
	{
		TSharedPtr<FCommandItem> Selected = SelectedItems[0];
		RemoveCommand(Selected->DescriptionText);
		CommandListView->RequestListRefresh();
		FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	}

	return FReply::Handled();
}

TOptional<FKey> SQuickCommandsPanel::GetCurrentKey(const TSharedPtr<FCommandItem>& Item) const
{
	if (Item.IsValid() && Item->BindKey.IsValid())
	{
		return Item->BindKey;
	}
	return Invalid;
}

void SQuickCommandsPanel::OnKeyChanged(const TSharedPtr<FKey>& SelectedKey, const TSharedPtr<FCommandItem>& Item)
{
	if (!Item.IsValid()) return;

	if (SelectedKey.IsValid())
	{
		Item->BindKey = *SelectedKey;
		Item->BindKeyString = SelectedKey->ToString();
	}
	else
	{
		Item->BindKey = EKeys::Invalid;
		Item->BindKeyString = TEXT("");
	}

	FQuickAccessToolModule::QuickAccessArchiveInfo.ChangeCommandKey(Item->DescriptionText, Item->BindKey);
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickCommandsPanel::EventOnKeyDown(const FKey& InKey) const
{
	if (FQuickAccessToolModule::QuickAccessArchiveInfo.CommandKey.Num() <= 0) return;

	for (auto KeyPair : FQuickAccessToolModule::QuickAccessArchiveInfo.CommandKey)
	{
		if (KeyPair.Value == InKey.ToString())
		{
			if (const FString* Command = FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap.Find(KeyPair.Key))
			{
				if (!Command->IsEmpty())
				{
					ExecuteCommand(*Command);
				}
			}
		}
	}
}

void SQuickCommandsPanel::AddCommand(const FString& CommandName, const FString& CommandText)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.AddCommand(CommandText, CommandName, CurrentKey);
	RefreshDisplayItems();
}

void SQuickCommandsPanel::RemoveCommand(const FString& Description)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.RemoveCommand(Description);
	RefreshDisplayItems();
}

void SQuickCommandsPanel::RemoveCommand(int32 Index)
{
	if (CommandItems.IsValidIndex(Index))
	{
		TSharedPtr<FCommandItem> ItemToRemove = CommandItems[Index];
		RemoveCommand(ItemToRemove->DescriptionText);
	}
}

void SQuickCommandsPanel::ClearAllCommands()
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.EmptyCommand();
	CommandItems.Empty();
	if (CommandListView.IsValid())
	{
		CommandListView->RequestListRefresh();
	}
	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SQuickCommandsPanel::RefreshDisplayItems()
{
	CommandItems.Empty();

	for (const auto& CommandPair : FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap)
	{
		FString* KeyName = FQuickAccessToolModule::QuickAccessArchiveInfo.CommandKey.Find(CommandPair.Key);
		TSharedPtr<FCommandItem> NewItem = MakeShared<FCommandItem>(CommandPair.Value, CommandPair.Key, KeyName ? *KeyName : FString());
		CommandItems.Add(NewItem);
	}

	if (CommandListView.IsValid())
	{
		CommandListView->RequestListRefresh();
	}
}
