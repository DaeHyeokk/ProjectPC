// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCUnitTakenDamageText.h"

#include "BaseGameplayTags.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPCUnitTakenDamageText::InitializeDamageText(const float DamageValue, const bool bIsCritical,
                                                  const FGameplayTag& DamageTypeTag)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageValue)));

		if (DamageTypeTag.MatchesTagExact(UnitGameplayTags::Unit_DamageType_Physical))
		{
			DamageText->SetColorAndOpacity(FSlateColor(PhysicalDamageColor));	
		}
		else if (DamageTypeTag.MatchesTagExact(UnitGameplayTags::Unit_DamageType_Magic))
		{
			DamageText->SetColorAndOpacity(FSlateColor(MagicDamageColor));
		}
	}

	if (CriticalImage)
		CriticalImage->SetVisibility(bIsCritical ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (DamageAnim)
		PlayAnimation(DamageAnim);
}
