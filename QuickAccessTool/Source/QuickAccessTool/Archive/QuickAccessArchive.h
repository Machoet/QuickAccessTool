// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuickAccessSettings.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

struct FQuickAccessArchiveInfo
{
	TArray<FString> PathArray = {};
	
	TMap<FString, FString> CommandMap = {};

	bool bCopyColorToClipboard = false;

	int32 ActiveMenuIndex = 0;

	FText CustomTaskText;
	
	int32 CustomTaskFontSize = 10;

	bool Save() const
	{
		const TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

		TArray<TSharedPtr<FJsonValue>> JsonArray;
		for (const FString& Path : PathArray)
		{
			JsonArray.Add(MakeShared<FJsonValueString>(Path));
		}
		
		TSharedPtr<FJsonObject> CommandMapObject = MakeShared<FJsonObject>();
		for (const auto& CommandPair : CommandMap)
		{
			CommandMapObject->SetStringField(CommandPair.Key, CommandPair.Value);
		}
		RootObject->SetObjectField("CommandMap", CommandMapObject);
		
		RootObject->SetArrayField("PathArray", JsonArray);
		RootObject->SetBoolField("bCopyColorToClipboard", bCopyColorToClipboard);
		RootObject->SetNumberField("ActiveMenuIndex", ActiveMenuIndex);
		RootObject->SetStringField("CustomTaskText", CustomTaskText.ToString());
		RootObject->SetNumberField("CustomTaskFontSize", CustomTaskFontSize);

		FString OutputString;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
		
		return FFileHelper::SaveStringToFile(OutputString, *UQuickAccessSettings::Get()->GetFilePath());
	}

	bool Load()
	{
		FString FileContent;
		if (!FFileHelper::LoadFileToString(FileContent, *UQuickAccessSettings::Get()->GetFilePath()))
		{
			return false;
		}

		TSharedPtr<FJsonObject> RootObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			return false;
		}
		PathArray.Empty();
		const TArray<TSharedPtr<FJsonValue>>* JsonArray;
		if (RootObject->TryGetArrayField("PathArray", JsonArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
			{
				if (Value->Type == EJson::String)
				{
					PathArray.Add(Value->AsString());
				}
			}
		}

		CommandMap.Empty();
		const TSharedPtr<FJsonObject>* CommandMapObject;
		if (RootObject->TryGetObjectField("CommandMap", CommandMapObject))
		{
			for (const auto& Field : (*CommandMapObject)->Values)
			{
				if (Field.Value->Type == EJson::String)
				{
					CommandMap.Add(Field.Key, Field.Value->AsString());
				}
			}
		}
		
		RootObject->TryGetBoolField("bCopyColorToClipboard", bCopyColorToClipboard);
		RootObject->TryGetNumberField("ActiveMenuIndex", ActiveMenuIndex);
		FString TempCustomTaskText;
		RootObject->TryGetStringField("CustomTaskText", TempCustomTaskText);
		CustomTaskText = FText::FromString(TempCustomTaskText);
		RootObject->TryGetNumberField("CustomTaskFontSize", CustomTaskFontSize);
		return true;
	}

	void AddCommand(const FString& Description, const FString& Command)
	{
		CommandMap.Add(Description, Command);
	}
	
	void RemoveCommand(const FString& Description)
	{
		CommandMap.Remove(Description);
	}
	
	bool HasCommand(const FString& Description) const
	{
		return CommandMap.Contains(Description);
	}
	
	TArray<FString> GetAllCommandDescriptions() const
	{
		TArray<FString> Descriptions;
		CommandMap.GetKeys(Descriptions);
		return Descriptions;
	}
	
	FString GetCommandByDescription(const FString& Description) const
	{
		const FString* Command = CommandMap.Find(Description);
		return Command ? *Command : FString();
	}
};
