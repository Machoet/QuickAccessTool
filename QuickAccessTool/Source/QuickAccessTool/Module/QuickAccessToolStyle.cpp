// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickAccessToolStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FQuickAccessToolStyle::StyleInstance = nullptr;

void FQuickAccessToolStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FQuickAccessToolStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FQuickAccessToolStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("QuickAccessToolStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16X16(16.0f, 16.0f);
const FVector2D Icon20X20(20.0f, 20.0f);
const FVector2D Icon40X40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FQuickAccessToolStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("QuickAccessToolStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("QuickAccessTool")->GetBaseDir() / TEXT("Resources"));

	Style->Set("QuickAccessTool.OpenQuickAccessTool", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40X40));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FQuickAccessToolStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FQuickAccessToolStyle::Get()
{
	return *StyleInstance;
}
