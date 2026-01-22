// CustomCommandsUI.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Input/Events.h"

struct FCommandItem;

class QUICKACCESSTOOL_API SQuickCommandsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickCommandsPanel)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void AddCommand(const FString& CommandName, const FString& CommandText);

	void RemoveCommand(const FString& Description);

	void RemoveCommand(int32 Index);

	void ClearAllCommands();

	void RefreshDisplayItems();

	virtual void OnKeyChanged(const TSharedPtr<FKey>& SelectedKey, const TSharedPtr<FCommandItem>& Item);

	virtual void EventOnKeyDown(const FKey& InKey) const;

private:
	TArray<TSharedPtr<FCommandItem>> CommandItems;

	TSharedPtr<SListView<TSharedPtr<FCommandItem>>> CommandListView;

	TSharedPtr<SEditableTextBox> NewCommandInput;

	TSharedPtr<SEditableTextBox> NewDescriptionInput;

	TSharedRef<ITableRow> OnGenerateCommandRow(TSharedPtr<FCommandItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void OnCommandItemClicked(TSharedPtr<FCommandItem> Item);

	static void ExecuteCommand(const FString& Command);

	FReply OnAddButtonClicked();

	FReply OnDeleteButtonClicked();

	TOptional<FKey> GetCurrentKey(const TSharedPtr<FCommandItem>& Item) const;

	FKey Invalid = EKeys::Invalid;

	FKey CurrentKey = EKeys::Invalid;
};
