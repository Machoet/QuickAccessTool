// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuickAccessSettings.generated.h"

/**
 * 
 */
UCLASS(Config=EditorPerProjectUserSettings)
class QUICKACCESSTOOL_API UQuickAccessSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "QuickAccessSettings")
	FDirectoryPath QuickAccessSavePath;

	FString GetFilePath() const
	{
		if (QuickAccessSavePath.Path.IsEmpty() || !FPaths::DirectoryExists(QuickAccessSavePath.Path))
		{
			return FPaths::ProjectSavedDir() / "QuickAccessData.json";
		}
		return QuickAccessSavePath.Path / "QuickAccessData.json";
	}

	static UQuickAccessSettings* Get() { return CastChecked<UQuickAccessSettings>(UQuickAccessSettings::StaticClass()->GetDefaultObject()); }

protected:
	FString GetLastFilePath() const
	{
		if (LastPath.IsEmpty() || !FPaths::DirectoryExists(LastPath))
		{
			return FPaths::ProjectSavedDir() / "QuickAccessData.json";
		}
		return LastPath / "QuickAccessData.json";
	}

	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
		if (QuickAccessSavePath.Path.IsEmpty() || !FPaths::DirectoryExists(QuickAccessSavePath.Path))
		{
			QuickAccessSavePath.Path = FPaths::ProjectSavedDir();
			LastPath = QuickAccessSavePath.Path;
		}
	}

	virtual void PreEditChange(FProperty* PropertyAboutToChange) override
	{
		Super::PreEditChange(PropertyAboutToChange);
		LastPath = QuickAccessSavePath.Path;
	}

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		if (LastPath != QuickAccessSavePath.Path)
		{
			MoveCacheFile(GetLastFilePath(), GetFilePath());
			LastPath = QuickAccessSavePath.Path;
		}
	}

	static bool MoveCacheFile(const FString& FromPath, const FString& ToPath)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.FileExists(*FromPath))
		{
			return true;
		}
		if (PlatformFile.FileExists(*ToPath))
		{
			const FString BackupPath = ToPath + TEXT(".backup");
			PlatformFile.CopyFile(*BackupPath, *ToPath);
		}

		const FString ToDirectory = FPaths::GetPath(ToPath);
		PlatformFile.CreateDirectoryTree(*ToDirectory);

		bool bSuccess = PlatformFile.MoveFile(*ToPath, *FromPath);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("Moved cache file: %s -> %s"), *FromPath, *ToPath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to move cache file: %s -> %s"), *FromPath, *ToPath);

			bSuccess = PlatformFile.CopyFile(*ToPath, *FromPath);
			if (bSuccess)
			{
				PlatformFile.DeleteFile(*FromPath);
				UE_LOG(LogTemp, Log, TEXT("Used copy+delete to move cache file"));
			}
		}

		return bSuccess;
	}

private:
	FString LastPath;
};
