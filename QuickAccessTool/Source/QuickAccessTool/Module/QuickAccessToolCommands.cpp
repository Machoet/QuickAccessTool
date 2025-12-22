// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickAccessToolCommands.h"

#define LOCTEXT_NAMESPACE "FQuickAccessToolModule"

void FQuickAccessToolCommands::RegisterCommands()
{
    UI_COMMAND(OpenQuickAccessTool, "QuickAccess", "File quick access tool", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Q));
    UI_COMMAND(PastePicture, "PastePicture", "Paste Picture", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::V));
}
#undef LOCTEXT_NAMESPACE
