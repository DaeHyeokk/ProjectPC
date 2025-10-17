// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCUnitCombatText.h"

#include "BaseGameplayTags.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPCUnitCombatText::InitializeDamageText(const float DamageValue, const bool bIsCritical,
                                                  const FGameplayTag& DamageTypeTag)
{
	if (ValueText)
	{
		FSlateFontInfo FontInfo = ValueText->GetFont();
		FontInfo.Size = 19.f;
		ValueText->SetFont(FontInfo);
		
		ValueText->SetText(FText::AsNumber(FMath::RoundToInt(DamageValue)));
		
		if (DamageTypeTag.MatchesTagExact(UnitGameplayTags::Unit_CombatText_Damage_Physical))
		{
			ValueText->SetColorAndOpacity(FSlateColor(PhysicalDamageColor));	
		}
		else if (DamageTypeTag.MatchesTagExact(UnitGameplayTags::Unit_CombatText_Damage_Magic))
		{
			ValueText->SetColorAndOpacity(FSlateColor(MagicDamageColor));
		}
		else if (DamageTypeTag.MatchesTagExact(UnitGameplayTags::Unit_CombatText_Damage_TrueDamage))
		{
			ValueText->SetColorAndOpacity(FSlateColor(TrueDamageColor));
		}
	}

	if (CriticalImage)
		CriticalImage->SetVisibility(bIsCritical ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (DamageAnim)
		PlayAnimation(DamageAnim);
}

void UPCUnitCombatText::InitializeHealText(const float HealValue)
{
	if (ValueText)
	{
		FSlateFontInfo FontInfo = ValueText->GetFont();
		FontInfo.Size = 13.f;
		ValueText->SetFont(FontInfo);

		const FString HealString = FString::Printf(TEXT("+%d"), FMath::RoundToInt(HealValue));
		ValueText->SetText(FText::FromString(HealString));
		ValueText->SetColorAndOpacity(FSlateColor(HealColor));
	}

	if (CriticalImage)
		CriticalImage->SetVisibility(ESlateVisibility::Collapsed);

	if (HealAnim)
		PlayAnimation(HealAnim);
}
