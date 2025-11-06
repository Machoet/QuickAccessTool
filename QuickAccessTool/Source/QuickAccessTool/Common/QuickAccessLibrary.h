// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuickAccessLibrary.generated.h"

/**
 * 
 */
UCLASS()
class QUICKACCESSTOOL_API UQuickAccessLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UTexture2D* SaveClipboardToAsset(FString AssetName = FString("NewTexture2D"));
	UFUNCTION(BlueprintCallable)
	static UTexture2D* CreateTextureAsset(const FString& PackagePath, const FString& AssetName, const int32 Width, const int32 Height, const TArray<FColor>& Pixels);
};
