// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuickAccessSettings.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

struct FCommandItem
{
	FString CommandName;
	FString CommandText;
	FKey BindKey;
	FString BindKeyString;

	FCommandItem(const FString& InName, const FString& InText)
		: CommandName(InName)
		  , CommandText(InText)
		  , BindKey(EKeys::Invalid)
	{
		BindKeyString = TEXT("");
	}

	FCommandItem(const FString& InName, const FString& InText, const FKey& InKey)
		: CommandName(InName)
		  , CommandText(InText)
		  , BindKey(InKey)
	{
		BindKeyString = InKey.ToString();
	}

	FCommandItem(const FString& InName, const FString& InText, const FString& InKey)
		: CommandName(InName)
		  , CommandText(InText),
		  BindKey(FName(InKey))
	{
		BindKeyString = InKey;
	}
};

struct FQuickAccessArchiveInfo
{
	TArray<FString> PathArray = {};

	TMap<FString, FString> CommandMap = {};

	TMap<FString, FString> CommandKey = {};

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
		RootObject->SetObjectField(TEXT("CommandMap"), CommandMapObject);

		TSharedPtr<FJsonObject> CommandKeyMapObject = MakeShared<FJsonObject>();
		for (const auto& CommandPair : CommandKey)
		{
			CommandKeyMapObject->SetStringField(CommandPair.Key, CommandPair.Value);
		}
		RootObject->SetObjectField(TEXT("CommandKey"), CommandKeyMapObject);

		RootObject->SetArrayField(TEXT("PathArray"), JsonArray);
		RootObject->SetBoolField(TEXT("bCopyColorToClipboard"), bCopyColorToClipboard);
		RootObject->SetNumberField(TEXT("ActiveMenuIndex"), ActiveMenuIndex);
		RootObject->SetStringField(TEXT("CustomTaskText"), CustomTaskText.ToString());
		RootObject->SetNumberField(TEXT("CustomTaskFontSize"), CustomTaskFontSize);

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
		if (RootObject->TryGetArrayField(TEXT("PathArray"), JsonArray))
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
		if (RootObject->TryGetObjectField(TEXT("CommandMap"), CommandMapObject))
		{
			for (const auto& Field : (*CommandMapObject)->Values)
			{
				if (Field.Value->Type == EJson::String)
				{
					CommandMap.Add(Field.Key, Field.Value->AsString());
				}
			}
		}

		CommandKey.Empty();
		const TSharedPtr<FJsonObject>* CommandKyeObject;
		if (RootObject->TryGetObjectField(TEXT("CommandKey"), CommandKyeObject))
		{
			for (const auto& Field : (*CommandKyeObject)->Values)
			{
				if (Field.Value->Type == EJson::String)
				{
					CommandKey.Add(Field.Key, Field.Value->AsString());
				}
			}
		}

		RootObject->TryGetBoolField(TEXT("bCopyColorToClipboard"), bCopyColorToClipboard);
		RootObject->TryGetNumberField(TEXT("ActiveMenuIndex"), ActiveMenuIndex);
		FString TempCustomTaskText;
		RootObject->TryGetStringField(TEXT("CustomTaskText"), TempCustomTaskText);
		CustomTaskText = FText::FromString(TempCustomTaskText);
		RootObject->TryGetNumberField(TEXT("CustomTaskFontSize"), CustomTaskFontSize);
		return true;
	}

	void AddCommand(const FString& Description, const FString& Command, const FKey& CurrentKey)
	{
		CommandMap.Add(Description, Command);
		CommandKey.Add(Description, CurrentKey.ToString());
	}

	void ChangeCommandKey(const FString& Description, const FKey& CurrentKey)
	{
		CommandKey.Add(Description, CurrentKey.ToString());
	}

	void RemoveCommand(const FString& Description)
	{
		CommandMap.Remove(Description);
		CommandKey.Remove(Description);
	}

	void EmptyCommand()
	{
		CommandMap.Empty();
		CommandKey.Empty();
	}
};
