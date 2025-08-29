#pragma once
#include "Widgets/Notifications/SProgressBar.h"

struct FUnitTickStyle
{
	float SegmentSize = 1000.f;
	int32 MajorEvery = 5;
	float MinPixelPerSeg = 8.f;
	FLinearColor MinorColor = FLinearColor::Black;
	FLinearColor MajorColor = FLinearColor::Black;
	float MinorThickness = 1.f;
	float MajorThickness = 1.f;
	FMargin Pad = FMargin(2.f);
};

class PCSUnitHealthProgressBar : public SProgressBar
{
public:
	SLATE_BEGIN_ARGS(PCSUnitHealthProgressBar)
		: _Percent(TOptional<float>(0.f)), _MaxValue(1.f) {}
		SLATE_ATTRIBUTE(TOptional<float>, Percent)
		SLATE_ATTRIBUTE(float, MaxValue)
		SLATE_ARGUMENT(FUnitTickStyle, TickStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetTickStyle(const FUnitTickStyle& In);
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TAttribute<TOptional<float>> PercentAttr;
	TAttribute<float> MaxValueAttr;
	FUnitTickStyle TickStyle;
};