// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "SUTScaleBox.h"

#if !UE_SERVER

void SUTScaleBox::Construct(const SUTScaleBox::FArguments& InArgs)
{
	bMaintainAspectRatio = InArgs._bMaintainAspectRatio;
	bFullScreen = InArgs._bFullScreen;

	ChildSlot
		[
			InArgs._Content.Widget
		];
}

void SUTScaleBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const EVisibility ChildVisibility = ChildSlot.GetWidget()->GetVisibility();
	if (ArrangedChildren.Accepts(ChildVisibility))
	{
		FVector2D DesiredSize = ChildSlot.GetWidget()->GetDesiredSize();
		FVector2D FinalOffset(0, 0);

		if (bMaintainAspectRatio)
		{

			if (AllottedGeometry.Size != DesiredSize)
			{
				float Aspect = DesiredSize.X / DesiredSize.Y;
				float Scale = AllottedGeometry.Size.Y / (bFullScreen ? 720.0f : DesiredSize.Y);
				DesiredSize.Y *= Scale;
				DesiredSize.X = DesiredSize.Y * Aspect;

				if (DesiredSize.X > AllottedGeometry.Size.X)
				{
					Aspect = DesiredSize.Y / DesiredSize.X;
					DesiredSize.X = AllottedGeometry.Size.X;
					DesiredSize.Y = DesiredSize.X * Aspect;
				}

				FinalOffset.X = AllottedGeometry.Size.X * 0.5 - DesiredSize.X * 0.5;
				FinalOffset.Y = AllottedGeometry.Size.Y * 0.5 - DesiredSize.Y * 0.5;
			}
		}
		else
		{
			DesiredSize = AllottedGeometry.Size;
		}

		float FinalScale = 1;

		ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(
			ChildSlot.GetWidget(),
			FinalOffset,
			DesiredSize,
			FinalScale
			));
	}
}


int32 SUTScaleBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SUTScaleBox::SetContent(TSharedRef<SWidget> InContent)
{
	ChildSlot
		[
			InContent
		];
}

#endif