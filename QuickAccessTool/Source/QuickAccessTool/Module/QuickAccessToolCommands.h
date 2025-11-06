// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "QuickAccessToolStyle.h"

class FQuickAccessToolCommands : public TCommands<FQuickAccessToolCommands>
{
public:

	FQuickAccessToolCommands()
		: TCommands<FQuickAccessToolCommands>(TEXT("QuickAccessTool"), NSLOCTEXT("Contexts", "QuickAccessTool", "QuickAccessTool Plugin"), NAME_None, FQuickAccessToolStyle::GetStyleSetName())
	{
	}
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenQuickAccessTool = nullptr;
	TSharedPtr<FUICommandInfo> PastePicture = nullptr;
};
