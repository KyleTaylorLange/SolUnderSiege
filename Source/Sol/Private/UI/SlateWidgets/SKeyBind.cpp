// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SKeyBind.h"

/*
void SKeyBind::Construct(const FArguments& InArgs)
{
	SButton::Construct(SButton::FArguments()
		.ButtonStyle(InArgs._ButtonStyle)
		.TextStyle(InArgs._TextStyle)
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.ContentPadding(InArgs._ContentPadding)
		.DesiredSizeScale(InArgs._DesiredSizeScale)
		.ContentScale(InArgs._ContentScale)
		.ButtonColorAndOpacity(InArgs._ButtonColorAndOpacity)
		.ForegroundColor(InArgs._ForegroundColor)
		);

	DefaultKey = InArgs._DefaultKey;
	OnKeyBindingChanged = InArgs._OnKeyBindingChanged;

	ChildSlot
		[
			SAssignNew(KeyText, STextBlock)
			.TextStyle(InArgs._TextStyle)
		];
	if (InArgs._Key.IsValid())
	{
		Key = InArgs._Key;
		KeyText->SetText(*Key == FKey() ? FString() : Key->ToString());
	}
	bWaitingForKey = false;
}

void SKeyBind::SetKey(FKey NewKey, bool bCanReset, bool bNotify)
{
	if (Key.IsValid())
	{
		FKey CurrentKey = *Key;
		if (NewKey == *Key && bCanReset)
		{
			NewKey = FKey();
		}
		*Key = NewKey;
		KeyText->SetText(NewKey == FKey() ? FString() : NewKey.ToString());
		KeyText->SetColorAndOpacity(NewKey == DefaultKey ? FLinearColor::Black : FLinearColor::White);
		bWaitingForKey = false;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(true);
		FSlateApplication::Get().ClearKeyboardFocus(EKeyboardFocusCause::SetDirectly);

		if (bNotify)
		{
			OnKeyBindingChanged.ExecuteIfBound(CurrentKey, NewKey);
		}
	}
}

FReply SKeyBind::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (bWaitingForKey)
	{
		SetKey(InKeyEvent.GetKey());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SKeyBind::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitingForKey)
	{
		SetKey(MouseEvent.GetEffectingButton());
		return FReply::Handled();
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//Get the center of the widget ao we can lock our mouse there
		FSlateRect Rect = MyGeometry.GetClippingRect();
		WaitingMousePos.X = (Rect.Left + Rect.Right) * 0.5f;
		WaitingMousePos.Y = (Rect.Top + Rect.Bottom) * 0.5f;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);

		KeyText->SetText(NSLOCTEXT("SKeyBind", "PressAnyKey", "** Press Any Button **"));
		KeyText->SetColorAndOpacity(FLinearColor(FColor(0, 0, 255, 255)));
		bWaitingForKey = true;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(false);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SKeyBind::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitingForKey)
	{
		SetKey(MouseEvent.GetWheelDelta() > 0 ? EKeys::MouseScrollUp : EKeys::MouseScrollDown);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

int32 SKeyBind::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//Make sure the mouse pointer doesnt leave the button
	if (bWaitingForKey)
	{
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);
	}
	return SButton::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}
*/

/*
FVector2D SKeyBind::ComputeDesiredSize() const
{
	return FVector2D(200.0f, 46.0f);
}
*/