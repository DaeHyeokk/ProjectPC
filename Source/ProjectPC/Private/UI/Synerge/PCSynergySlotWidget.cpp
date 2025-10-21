// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Synerge/PCSynergySlotWidget.h"

#include "Component/PCSynergyComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataAsset/UI/PCSynergyInfoData.h"


void UPCSynergySlotWidget::SetData(const FSynergyData& InData)
{
	FPCSynergyUIRow UIInfo;
	const bool bHasUI = (SynergyUIData && SynergyUIData->Resolve(InData.SynergyTag, UIInfo));

	if (SynergyName)
	{
		FText Name = bHasUI ? UIInfo.DisplayName : FText::FromName(InData.SynergyTag.GetTagName());
		SynergyName->SetText(Name);
	}

	if (SynergyIcon)
	{
		if (bHasUI)
		{
			SynergyIcon->SetBrush(UIInfo.Icon);

			if (InData.TierIndex >= 0)
			{
				SynergyIcon->SetColorAndOpacity(FLinearColor::Black);
			}
		}
	}

	if (SynergyCount)
	{
		SynergyCount->SetText(FText::AsNumber(InData.Count));
	}

	auto SetNumOrEmpty = [](UTextBlock* W, int32 Val)
	{
		if (!W) return;
		if (Val >= 0) W->SetText(FText::AsNumber(Val));
		else          W->SetText(FText::GetEmpty()); // 공백
	};

	if (TierThresholds1 && TierThresholds2 && TierThresholds3 && TierThresholds4)
	{
		if (InData.TierIndex == -1)
		{
			// 아직 티어 없음 → Count / 첫 임계값
			TierThresholds1->SetText(FText::AsNumber(InData.Count));
			SetNumOrEmpty(TierThresholds2, InData.Thresholds.IsValidIndex(0) ? InData.Thresholds[0] : -1);
			TierThresholds3->SetText(FText::GetEmpty());
			TierThresholds4->SetText(FText::GetEmpty());
			TierSlash1->SetText(FText::GetEmpty());
			TierSlash2->SetText(FText::GetEmpty());
		}
		else
		{
			// 티어 있음 → 전체 임계값 보여주기
			SetNumOrEmpty(TierThresholds1, InData.Thresholds.IsValidIndex(0) ? InData.Thresholds[0] : -1);
			SetNumOrEmpty(TierThresholds2, InData.Thresholds.IsValidIndex(1) ? InData.Thresholds[1] : -1);
			SetNumOrEmpty(TierThresholds3, InData.Thresholds.IsValidIndex(2) ? InData.Thresholds[2] : -1);
			SetNumOrEmpty(TierThresholds4, InData.Thresholds.IsValidIndex(3) ? InData.Thresholds[3] : -1);

			if (InData.Thresholds.IsValidIndex(2) && InData.Thresholds.IsValidIndex(3))
			{
				TierSlash2->SetText(FText::FromString("/"));
			}
			else
			{
				TierSlash2->SetText(FText::GetEmpty());
			}
			
		}
	}

	if (TierIcon)
	{
		if (bHasUI && UIInfo.TierSet.TierBorders.Num() > 0)
		{
			const int32 TierIdx = InData.TierIndex;
			const FSlateBrush& Brush = UIInfo.TierSet.TierBorders.IsValidIndex(TierIdx) ? UIInfo.TierSet.TierBorders[TierIdx+1] : UIInfo.TierSet.TierBorders[0];

			TierIcon->SetBrush(Brush);
		}
	}
}
