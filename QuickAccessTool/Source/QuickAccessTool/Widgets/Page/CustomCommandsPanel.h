// CustomCommandsUI.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FCommandItem
{
	FString CommandName;
	FString CommandText;
    
	FCommandItem(const FString& InName, const FString& InText)
		: CommandName(InName)
		, CommandText(InText)
	{}
};

class QUICKACCESSTOOL_API SCustomCommandsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomCommandsPanel)
	{}
    
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
    
	void AddCommand(const FString& CommandName, const FString& CommandText);
	
	void RemoveCommand(const FString& Description);

	void RemoveCommand(int32 Index);
    
	void ClearAllCommands();
	
	void RefreshDisplayItems();

private:
	TArray<TSharedPtr<FCommandItem>> CommandItems;
    
	TSharedPtr<SListView<TSharedPtr<FCommandItem>>> CommandListView;
    
	TSharedPtr<SEditableTextBox> NewCommandInput;
    
	TSharedPtr<SEditableTextBox> NewDescriptionInput;
    
	TSharedRef<ITableRow> OnGenerateCommandRow(TSharedPtr<FCommandItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
    
	void OnCommandItemClicked(TSharedPtr<FCommandItem> Item);
    
	FReply OnAddButtonClicked();
	
	FReply OnDeleteButtonClicked();
	
	static void ExecuteConsoleCommand(const FString& Command);
};