#pragma once
#include "Framework/Application/IInputProcessor.h"
#include "Math/Vector2D.h"
#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuickAccessKeyEvent, const FKey&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQuickAccessKeyAnalogEvent, const FKey&, float);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQuickAccessMouseMoveEvent, const FVector2D&, const FVector2D&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuickAccessMouseWheelEvent, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuickAccessDoubleClickEvent, const FKey&);


struct FInputKeyParams;

class FQuickAccessInputProcessor : public IInputProcessor
{
public:
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;

	virtual const TCHAR* GetDebugName() const override { return TEXT("QuickAccessInputProcessor"); }

	FOnQuickAccessKeyEvent OnKeyDown;

	FOnQuickAccessKeyEvent OnKeyUp;

	FOnQuickAccessKeyAnalogEvent OnAnalogInput;

	FOnQuickAccessMouseMoveEvent OnMouseMoveEvent;
	
	FOnQuickAccessMouseWheelEvent OnMouseWheelEvent;

	FOnQuickAccessDoubleClickEvent OnDoubleClickEvent;

	FOnQuickAccessKeyEvent KeyEvent;
};
