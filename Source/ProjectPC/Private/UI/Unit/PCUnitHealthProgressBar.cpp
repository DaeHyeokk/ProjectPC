// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCUnitHealthProgressBar.h"

#include "UI/Unit/PCSUnitHealthProgressBar.h"


void UPCUnitHealthProgressBar::SetValues(float InCurrent, float InMax)
{
	MaxValue = FMath::Max(1.f, InMax);
	SetPercent(InMax > 0.f ? InCurrent / MaxValue : 0.f);

	InvalidateLayoutAndVolatility();
}

TSharedRef<SWidget> UPCUnitHealthProgressBar::RebuildWidget()
{
	Super::RebuildWidget();

	const FUnitTickStyle Style;

	UnitProgressSlate =
		SNew(PCSUnitHealthProgressBar)
		.Percent(TAttribute<TOptional<float>>::Create(
			TAttribute<TOptional<float>>::FGetter::CreateUObject(this, &ThisClass::GetOptionalPercent)))
		.MaxValue(TAttribute<float>::Create(
			TAttribute<float>::FGetter::CreateUObject(this, &ThisClass::GetMaxValueAttr)))
		.TickStyle(Style);

	MyProgressBar = UnitProgressSlate.ToSharedRef();
	Super::SynchronizeProperties();
	
	return UnitProgressSlate.ToSharedRef();
}

void UPCUnitHealthProgressBar::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	UnitProgressSlate.Reset();
}

void UPCUnitHealthProgressBar::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (UnitProgressSlate.IsValid())
	{
		UnitProgressSlate->SetTickStyle(MakeSlateTickStyle());
	}

	InvalidateLayoutAndVolatility();
}

FUnitTickStyle UPCUnitHealthProgressBar::MakeSlateTickStyle() const
{
	FUnitTickStyle Out;
	Out.SegmentSize     = TickStyleSettings.SegmentSize;
	Out.MajorEvery      = TickStyleSettings.MajorEvery;
	Out.MinPixelPerSeg = TickStyleSettings.MinPixelPerSeg;
	Out.MinorColor      = TickStyleSettings.MinorColor;
	Out.MajorColor      = TickStyleSettings.MajorColor;
	Out.MinorThickness  = TickStyleSettings.MinorThickness;
	Out.MajorThickness  = TickStyleSettings.MajorThickness;
	Out.Pad             = TickStyleSettings.Padding;

	return Out;
}
