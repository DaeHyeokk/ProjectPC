// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "PCUnitHealthProgressBar.generated.h"

struct FUnitTickStyle;
class PCSUnitProgressBar;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FUnitTickStyleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Ticks", meta=(ClampMin="1", UIMin="1"))
	float SegmentSize = 1000.f;
	
	UPROPERTY(EditAnywhere, Category="Ticks", meta=(ClampMin="1", UIMin="1"))
	int32 MajorEvery = 5;

	UPROPERTY(EditAnywhere, Category="Ticks", meta=(ClampMin="0", UIMin="0"))
	float MinPixelPerSeg = 8.f;

	UPROPERTY(EditAnywhere, Category="Ticks")
	FLinearColor MinorColor = FLinearColor::Black;
	UPROPERTY(EditAnywhere, Category="Ticks")
	FLinearColor MajorColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, Category="Ticks", meta=(ClampMin="0"))
	float MinorThickness = 1.f;
	UPROPERTY(EditAnywhere, Category="Ticks", meta=(ClampMin="0"))
	float MajorThickness = 1.f;

	UPROPERTY(EditAnywhere, Category="Ticks")
	FMargin Padding = FMargin(2.f);
};

UCLASS(meta=(DisplayName="Unit Health Progress Bar"))
class PROJECTPC_API UPCUnitHealthProgressBar : public UProgressBar
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health Bar")
	float MaxValue = 1000.f; // 눈금 계산용 (체력 1000마다 눈금 한 개)

	UPROPERTY(EditAnywhere, Category="Health Bar|TickStyle", meta=(ShowOnlyInnerProperties))
	FUnitTickStyleSettings TickStyleSettings;
	
	UFUNCTION(BlueprintCallable, Category="Unit Health Bar")
	void SetValues(float InCurrent, float InMax);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

private:
	TSharedPtr<PCSUnitProgressBar> UnitProgressSlate;

	TOptional<float> GetOptionalPercent() const { return TOptional<float>(GetPercent()); }
	float GetMaxValueAttr() const { return MaxValue; }
	FUnitTickStyle MakeSlateTickStyle() const;
	
};
