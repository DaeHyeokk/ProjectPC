// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Synerge/PCSynergyInfoWidget.h"

#include "Component/PCSynergyComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "DataAsset/UI/PCSynergyInfoData.h"


void UPCSynergyInfoWidget::SetSynergyInfo(const FPCSynergyUIRow& UI, const FSynergyData& Runtime)
{
	if (SynergyIcon)
	{
		SynergyIcon->SetBrush(UI.Icon);
		SynergyIcon->SetBrushTintColor(EStyleColor::White);
	}

	if (SynergyName)
	{
		SynergyName->SetText(UI.DisplayName);
	}

	if (Summary)
	{
		Summary->SetText(UI.Summary);
	}

	if (SynergyDescription)
	{
		SynergyDescription->SetText(UI.Description);
	}

	if (TierEffectInfo)
	{
		TierEffectInfo->ClearChildren();
		const int32 CurrentCount = Runtime.Count;

		for (const auto& TierLine : UI.TierLines)
		{
			UTextBlock* Line = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
			Line->SetText(FText::Format(FText::FromString(TEXT("({0}) {1}")), FText::AsNumber(TierLine.Threshold), TierLine.Line));
			FSlateFontInfo FontInfo;
			FontInfo.Size = 14;
			FontInfo.FontObject = SynergyInfoFont;
			Line->SetFont(FontInfo);
			Line->SetAutoWrapText(true);

			if (CurrentCount >= TierLine.Threshold)
			{
				Line->SetOpacity(1.f);
			}
			else
			{
				Line->SetOpacity(0.5f);
			}
			TierEffectInfo->AddChild(Line);
		}
		
	}
}
