#include "UI/Unit/PCSUnitProgressBar.h"

void PCSUnitProgressBar::Construct(const FArguments& InArgs)
{
	PercentAttr = InArgs._Percent;
	MaxValueAttr = InArgs._MaxValue;
	TickStyle = InArgs._TickStyle;
	SProgressBar::Construct(SProgressBar::FArguments().Percent(InArgs._Percent));
}

void PCSUnitProgressBar::SetTickStyle(const FUnitTickStyle& In)
{
	TickStyle = In;
}

int32 PCSUnitProgressBar::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                  const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                  const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 AfterBar = SProgressBar::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const FMargin& P = TickStyle.Pad;
	const FVector2D TL(P.Left, P.Top);
	const FVector2D Inner(
		Size.X - P.GetTotalSpaceAlong<Orient_Horizontal>(),
		Size.Y - P.GetTotalSpaceAlong<Orient_Vertical>()
		);

	const FPaintGeometry BarGeo = AllottedGeometry.ToPaintGeometry(
		FVector2f((float)Inner.X, (float)Inner.Y),
		FSlateLayoutTransform(FVector2f((float)TL.X, (float)TL.Y)));
	
	// 최대값/세그 계산
	const float MaxV = FMath::Max(1.f, MaxValueAttr.Get());
	const float SegV = FMath::Max(1.f, TickStyle.SegmentSize);
	
	// 현재 퍼센트 / 채워진 픽셀폭
	const float Pct = FMath::Clamp(PercentAttr.Get().Get(0.f),0.f,1.f);
	const float FillW = Inner.X * Pct;

	// 현재 채워진 값(= 현재 체력)과 그 안에서 필요한 선 개수
	const int32 NumLines = FMath::FloorToInt((MaxV * Pct) / SegV);	// 1000, 2000, MaxV 미만
	const int32 NumFilled = FMath::FloorToInt((MaxV * Pct) / SegV);	// 현재 체력 이하까지만

	const FVector2f TLf(P.Left, P.Top);
	
	const FVector2D AbsTL = AllottedGeometry.LocalToAbsolute(TLf);
	const FVector2D AbsBR = AllottedGeometry.LocalToAbsolute(FVector2D(TLf.X + FillW, TLf.Y + Inner.Y));
//	OutDrawElements.PushClip(FSlateClippingZone(FSlateRect(AbsTL.X, AbsTL.Y, AbsBR.X, AbsBR.Y)));

	for (int32 i=1; i<=NumLines; ++i)
	{
		const float Alpha = (i * SegV) / MaxV;
		float X = Alpha * Inner.X;

		const bool bMajor = (TickStyle.MajorEvery > 0) && (i % TickStyle.MajorEvery == 0);
		const float Th = bMajor ? TickStyle.MajorThickness : TickStyle.MinorThickness;
		const FLinearColor LinearColor = bMajor ? TickStyle.MajorColor : TickStyle.MinorColor;
		
		X = FMath::RoundToFloat(X);
		
		FSlateDrawElement::MakeLines(
			OutDrawElements, AfterBar + 1,
			BarGeo,
			{ FVector2f(X, 0.f), FVector2f(X, (float)Inner.Y) },
			ESlateDrawEffect::None, LinearColor,false, Th
			);
	}

	//OutDrawElements.PopClip();
	
	return AfterBar + 2;
}
