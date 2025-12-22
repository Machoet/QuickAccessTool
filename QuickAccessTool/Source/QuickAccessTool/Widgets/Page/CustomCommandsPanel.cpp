#include "CustomCommandsPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/STableRow.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"
#include "QuickAccessTool/Language/Language.h"
#include "QuickAccessTool/Module/QuickAccessTool.h"

void SCustomCommandsPanel::Construct(const FArguments& InArgs)
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
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::AddButton)
					.OnClicked(this, &SCustomCommandsPanel::OnAddButtonClicked)
					.HAlign(HAlign_Center)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(QuickAccessToolLanguage::DeleteButton)
					.OnClicked(this, &SCustomCommandsPanel::OnDeleteButtonClicked)
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
				.OnGenerateRow(this, &SCustomCommandsPanel::OnGenerateCommandRow)
				.OnMouseButtonDoubleClick(this, &SCustomCommandsPanel::OnCommandItemClicked)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];
}

TSharedRef<ITableRow> SCustomCommandsPanel::OnGenerateCommandRow(
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
						.Text(FText::FromString(Item->CommandText))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ToolTipText(FText::FromString(Item->CommandName))
					]
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

void SCustomCommandsPanel::OnCommandItemClicked(TSharedPtr<FCommandItem> Item)
{
#if WITH_EDITOR
	if (GEditor)
	{
		if (GEditor->PlayWorld)
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(
				GEditor->PlayWorld,
				Item->CommandName,
				nullptr
			);
			return;
		}
		else
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(
				GEditor->GetEditorWorldContext().World(),
				Item->CommandName,
				nullptr
			);
			return;
		}
	}
#endif

	if (const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(World, Item->CommandName, nullptr);
	}
}

FReply SCustomCommandsPanel::OnAddButtonClicked()
{
	FString Command = NewCommandInput->GetText().ToString();
	FString Description = NewDescriptionInput->GetText().ToString();

	if (!Command.IsEmpty())
	{
		if (Description.IsEmpty())
		{
			Description = Command;
		}

		AddCommand(Command, Description);
		NewCommandInput->SetText(FText::GetEmpty());
		NewDescriptionInput->SetText(FText::GetEmpty());
		FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
	}

	return FReply::Handled();
}

FReply SCustomCommandsPanel::OnDeleteButtonClicked()
{
	if (CommandListView.IsValid())
	{
		TArray<TSharedPtr<FCommandItem>> SelectedItems = CommandListView->GetSelectedItems();
		if (SelectedItems.Num() > 0)
		{
			TSharedPtr<FCommandItem> SelectedItem = SelectedItems[0];
			RemoveCommand(SelectedItem->CommandText);
			CommandListView->RequestListRefresh();
			FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
		}
	}

	return FReply::Handled();
}

void SCustomCommandsPanel::AddCommand(const FString& CommandName, const FString& CommandText)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap.Add(CommandText, CommandName);
	RefreshDisplayItems();
}

void SCustomCommandsPanel::RemoveCommand(const FString& Description)
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap.Remove(Description);
	RefreshDisplayItems();
}

void SCustomCommandsPanel::RemoveCommand(int32 Index)
{
	if (CommandItems.IsValidIndex(Index))
	{
		TSharedPtr<FCommandItem> ItemToRemove = CommandItems[Index];
		RemoveCommand(ItemToRemove->CommandText);
	}
}

void SCustomCommandsPanel::ClearAllCommands()
{
	FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap.Empty();

	CommandItems.Empty();

	if (CommandListView.IsValid())
	{
		CommandListView->RequestListRefresh();
	}

	FQuickAccessToolModule::QuickAccessArchiveInfo.Save();
}

void SCustomCommandsPanel::ExecuteConsoleCommand(const FString& Command)
{
	if (GEngine)
	{
		GEngine->Exec(nullptr, *Command);
	}
}

void SCustomCommandsPanel::RefreshDisplayItems()
{
	CommandItems.Empty();

	for (const auto& CommandPair : FQuickAccessToolModule::QuickAccessArchiveInfo.CommandMap)
	{
		TSharedPtr<FCommandItem> NewItem = MakeShared<FCommandItem>(CommandPair.Value, CommandPair.Key);
		CommandItems.Add(NewItem);
	}

	if (CommandListView.IsValid())
	{
		CommandListView->RequestListRefresh();
	}
}
