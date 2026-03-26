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
    FString CommandText;
    FString DescriptionText;
    FKey BindKey;
    FString BindKeyString;

    FCommandItem(const FString& InName, const FString& InText)
       : CommandText(InName)
         , DescriptionText(InText)
         , BindKey(EKeys::Invalid)
    {
       BindKeyString = TEXT("");
    }

    FCommandItem(const FString& InName, const FString& InText, const FKey& InKey)
       : CommandText(InName)
         , DescriptionText(InText)
         , BindKey(InKey)
    {
       BindKeyString = InKey.ToString();
    }

    FCommandItem(const FString& InName, const FString& InText, const FString& InKey)
       : CommandText(InName)
         , DescriptionText(InText),
         BindKey(FName(InKey))
    {
       BindKeyString = InKey;
    }
};

struct FQuickAccessArchiveInfo
{
    TMap<FString, TArray<FString>> MultiPathMap = {};
    TMap<FString, FString> CommandMap = {};
    TMap<FString, FString> CommandKey = {};
    bool bCopyColorToClipboard = false;
    int32 ActiveMenuIndex = 0;
    int32 ActiveQuickPanelMenuIndex = 0;
    FText CustomTaskText;
    int32 CustomTaskFontSize = 10;

    bool Save() const
    {
       const TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

       const TSharedPtr<FJsonObject> MultiPathMapObject = MakeShared<FJsonObject>();
       for (const auto& Pair : MultiPathMap)
       {
          TArray<TSharedPtr<FJsonValue>> JsonArray;
          for (const FString& Path : Pair.Value)
          {
             JsonArray.Add(MakeShared<FJsonValueString>(Path));
          }
          MultiPathMapObject->SetArrayField(Pair.Key, JsonArray);
       }
       RootObject->SetObjectField(TEXT("MultiPathMap"), MultiPathMapObject);

       const TSharedPtr<FJsonObject> CommandMapObject = MakeShared<FJsonObject>();
       for (const auto& CommandPair : CommandMap)
       {
          CommandMapObject->SetStringField(CommandPair.Key, CommandPair.Value);
       }
       RootObject->SetObjectField(TEXT("CommandMap"), CommandMapObject);

       const TSharedPtr<FJsonObject> CommandKeyMapObject = MakeShared<FJsonObject>();
       for (const auto& CommandPair : CommandKey)
       {
          CommandKeyMapObject->SetStringField(CommandPair.Key, CommandPair.Value);
       }
       RootObject->SetObjectField(TEXT("CommandKey"), CommandKeyMapObject);

       RootObject->SetBoolField(TEXT("bCopyColorToClipboard"), bCopyColorToClipboard);
       RootObject->SetNumberField(TEXT("ActiveMenuIndex"), ActiveMenuIndex);
       RootObject->SetNumberField(TEXT("ActiveQuickPanelMenuIndex"), ActiveQuickPanelMenuIndex);
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

       MultiPathMap.Empty();
       const TSharedPtr<FJsonObject>* MultiPathMapObject;
       if (RootObject->TryGetObjectField(TEXT("MultiPathMap"), MultiPathMapObject))
       {
          for (const auto& Field : (*MultiPathMapObject)->Values)
          {
             TArray<FString> Paths;
             const TArray<TSharedPtr<FJsonValue>>* JsonArray;
             if (Field.Value->TryGetArray(JsonArray))
             {
                for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
                {
                   Paths.Add(Value->AsString());
                }
             }
             MultiPathMap.Add(Field.Key, Paths);
          }
       }
       // 兼容旧版 PathArray 数据
       else
       {
          const TArray<TSharedPtr<FJsonValue>>* OldArray;
          if (RootObject->TryGetArrayField(TEXT("PathArray"), OldArray))
          {
             TArray<FString> Paths;
             for (const auto& JsonValue : *OldArray) Paths.Add(JsonValue->AsString());
             MultiPathMap.Add(TEXT("Default"), Paths);
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
       RootObject->TryGetNumberField(TEXT("ActiveQuickPanelMenuIndex"), ActiveQuickPanelMenuIndex);
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