#include "QuickAccessInputProcessor.h"

void FQuickAccessInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FQuickAccessInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	OnKeyDown.Broadcast(InKeyEvent.GetKey());
	KeyEvent.Broadcast(InKeyEvent.GetKey());
	return IInputProcessor::HandleKeyDownEvent(SlateApp, InKeyEvent);
}

bool FQuickAccessInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	OnKeyUp.Broadcast(InKeyEvent.GetKey());
	return IInputProcessor::HandleKeyUpEvent(SlateApp, InKeyEvent);
}

bool FQuickAccessInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	OnAnalogInput.Broadcast(InAnalogInputEvent.GetKey(), InAnalogInputEvent.GetAnalogValue());
	KeyEvent.Broadcast(InAnalogInputEvent.GetKey());
	return IInputProcessor::HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
}

bool FQuickAccessInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	OnMouseMoveEvent.Broadcast(MouseEvent.GetScreenSpacePosition(), MouseEvent.GetLastScreenSpacePosition());
	return IInputProcessor::HandleMouseMoveEvent(SlateApp, MouseEvent);
}

bool FQuickAccessInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	OnKeyDown.Broadcast(MouseEvent.GetEffectingButton());
	KeyEvent.Broadcast(MouseEvent.GetEffectingButton());
	return IInputProcessor::HandleMouseButtonDownEvent(SlateApp, MouseEvent);
}

bool FQuickAccessInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	OnKeyUp.Broadcast(MouseEvent.GetEffectingButton());
	return IInputProcessor::HandleMouseButtonUpEvent(SlateApp, MouseEvent);
}

bool FQuickAccessInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	OnDoubleClickEvent.Broadcast(MouseEvent.GetEffectingButton());
	return IInputProcessor::HandleMouseButtonDoubleClickEvent(SlateApp, MouseEvent);
}

bool FQuickAccessInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
	OnMouseWheelEvent.Broadcast(InWheelEvent.GetWheelDelta());
	KeyEvent.Broadcast(InWheelEvent.GetEffectingButton());
	
	return IInputProcessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGestureEvent);
}
