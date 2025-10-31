#include "QuickAccessLibrary.h"
#include "FileHelpers.h"
#include "Engine/Texture2D.h"
#include "Framework/Notifications/NotificationManager.h"
#include "QuickAccessTool/Language/Language.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#endif

UTexture2D* UQuickAccessLibrary::SaveClipboardToAsset(FString AssetName)
{
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedPaths;
	ContentBrowser.GetSelectedPathViewFolders(SelectedPaths);

	if (SelectedPaths.Num() <= 0)
	{
		return nullptr;
	}
	FString LeftStr;
	FString RightStr;

	FString PackagePath;
	if (SelectedPaths[0].StartsWith(TEXT("/All/")))
	{
		SelectedPaths[0].Split(TEXT("/All/"), &LeftStr, &RightStr);
		PackagePath = TEXT("/") + RightStr + TEXT("/");
	}
	else
	{
		PackagePath = SelectedPaths[0] + TEXT("/");
	}

	if (PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		return nullptr;
	}

	PackagePath.Split(TEXT("/Game/"), &LeftStr, &RightStr);
	FString FullPath = FPaths::ProjectContentDir() + RightStr + AssetName + TEXT(".uasset");
	if (IFileManager::Get().FileExists(*FullPath))
	{
		int Index = 0;
		while (true)
		{
			PackagePath.Split(TEXT("/Game/"), &LeftStr, &RightStr);
			FullPath = FPaths::ProjectContentDir() + RightStr + AssetName + "_" + FString::FromInt(Index) + TEXT(".uasset");
			if (!IFileManager::Get().FileExists(*FullPath))
			{
				AssetName = AssetName + "_" + FString::FromInt(Index);
				break;
			}
			Index++;
		}
	}
#if PLATFORM_WINDOWS
	if (!OpenClipboard(nullptr))
	{
		return nullptr;
	}

	UTexture2D* ResultTexture = nullptr;
	HANDLE Handle = GetClipboardData(CF_DIB);
	HBITMAP hBitmap = nullptr;

	if (!Handle)
	{
		hBitmap = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));
		if (!hBitmap)
		{
			CloseClipboard();
			FSlateNotificationManager::Get().AddNotification(FNotificationInfo(QuickAccessToolLanguage::ClipboardNotHasPictures));
			return nullptr;
		}
	}

	TArray<FColor> Pixels;
	int32 Width = 0;
	int32 Height = 0;

	if (Handle)
	{
		BITMAPINFO* BitMapinfo = static_cast<BITMAPINFO*>(GlobalLock(Handle));
		if (!BitMapinfo)
		{
			CloseClipboard();
			return nullptr;
		}

		Width = BitMapinfo->bmiHeader.biWidth;
		Height = FMath::Abs(BitMapinfo->bmiHeader.biHeight);
		const uint8* Src = reinterpret_cast<uint8*>(BitMapinfo) + BitMapinfo->bmiHeader.biSize + BitMapinfo->bmiHeader.biClrUsed * sizeof(RGBQUAD);

		Pixels.SetNum(Width * Height);

		for (int32 y = 0; y < Height; ++y)
		{
			const uint8* Row = Src + (Height - 1 - y) * Width * 4;
			for (int32 x = 0; x < Width; ++x)
			{
				const uint8 B = Row[x * 4 + 0];
				const uint8 G = Row[x * 4 + 1];
				const uint8 R = Row[x * 4 + 2];
				Pixels[y * Width + x] = FColor(R, G, B, 255);
			}
		}

		GlobalUnlock(Handle);
	}
	else if (hBitmap)
	{
		BITMAP Bitmap = {};
		::GetObjectW((HGDIOBJ)hBitmap, sizeof(BITMAP), &Bitmap);

		const HDC TempHdc = GetDC(nullptr);

		BITMAPINFO Info = {};
		Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		Info.bmiHeader.biWidth = Bitmap.bmWidth;
		Info.bmiHeader.biHeight = -Bitmap.bmHeight;
		Info.bmiHeader.biPlanes = 1;
		Info.bmiHeader.biBitCount = 32;
		Info.bmiHeader.biCompression = BI_RGB;

		const int32 RowBytes = Bitmap.bmWidth * 4;
		TArray<uint8> RawData;
		RawData.SetNumZeroed(RowBytes * Bitmap.bmHeight);

		const int Ret = GetDIBits(TempHdc, hBitmap, 0, Bitmap.bmHeight, RawData.GetData(), &Info, DIB_RGB_COLORS);
		ReleaseDC(nullptr, TempHdc);

		if (Ret == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetDIBits failed"));
			CloseClipboard();
			return nullptr;
		}

		Width = Bitmap.bmWidth;
		Height = Bitmap.bmHeight;
		Pixels.SetNum(Width * Height);

		for (int y = 0; y < Height; ++y)
		{
			const uint8* Row = RawData.GetData() + y * RowBytes;
			for (int x = 0; x < Width; ++x)
			{
				const uint8 B = Row[x * 4 + 0];
				const uint8 G = Row[x * 4 + 1];
				const uint8 R = Row[x * 4 + 2];
				Pixels[y * Width + x] = FColor(R, G, B, 255);
			}
		}
	}

	CloseClipboard();

	if (Pixels.Num() > 0)
	{
		ResultTexture = CreateTextureAsset(PackagePath, AssetName, Width, Height, Pixels);
	}

	return ResultTexture;
#else
	return nullptr;
#endif
}

UTexture2D* UQuickAccessLibrary::CreateTextureAsset(const FString& PackagePath, const FString& AssetName, const int32 Width, const int32 Height, const TArray<FColor>& Pixels)
{
	const FString SanitizedPackagePath = PackagePath;
	if (SanitizedPackagePath.IsEmpty())
	{
		return nullptr;
	}

	const FString SanitizedAssetName = AssetName;
	if (SanitizedAssetName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackageName = SanitizedPackagePath + SanitizedAssetName;

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return nullptr;
	}

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *SanitizedAssetName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	if (!NewTexture)
	{
		return nullptr;
	}

	FTexturePlatformData* TexturePlatformData = new FTexturePlatformData();
	TexturePlatformData->SizeX = Width;
	TexturePlatformData->SizeY = Height;
	TexturePlatformData->PixelFormat = PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	TexturePlatformData->Mips.Add(Mip);
#if ENGINE_MAJOR_VERSION == 4
	NewTexture->PlatformData = TexturePlatformData;
#elif ENGINE_MAJOR_VERSION == 5
	NewTexture->SetPlatformData(TexturePlatformData);
#endif

	Mip->SizeX = Width;
	Mip->SizeY = Height;

	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* Data = Mip->BulkData.Realloc(Width * Height * 4);
	FMemory::Memcpy(Data, Pixels.GetData(), Width * Height * 4);
	Mip->BulkData.Unlock();

	NewTexture->UpdateResource();

	Package->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(NewTexture);

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);

	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
	return NewTexture;
}
