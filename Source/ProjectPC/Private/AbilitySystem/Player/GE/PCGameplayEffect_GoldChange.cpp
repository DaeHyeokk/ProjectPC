// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_GoldChange.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_GoldChange::UPCGameplayEffect_GoldChange()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo GoldModifier;
	GoldModifier.Attribute = UPCPlayerAttributeSet::GetPlayerGoldAttribute();
	GoldModifier.ModifierOp = EGameplayModOp::Additive;

	Modifiers.Add(GoldModifier);
}
