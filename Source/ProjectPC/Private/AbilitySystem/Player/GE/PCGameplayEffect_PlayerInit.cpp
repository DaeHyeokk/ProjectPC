// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_PlayerInit.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_PlayerInit::UPCGameplayEffect_PlayerInit()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo PlayerLevelModifier;
	PlayerLevelModifier.Attribute = UPCPlayerAttributeSet::GetPlayerLevelAttribute();
	PlayerLevelModifier.ModifierOp = EGameplayModOp::Override;
	PlayerLevelModifier.ModifierMagnitude = FScalableFloat(InitPlayerLevel);
	Modifiers.Add(PlayerLevelModifier);

	FGameplayModifierInfo PlayerXPModifier;
	PlayerXPModifier.Attribute = UPCPlayerAttributeSet::GetPlayerXPAttribute();
	PlayerXPModifier.ModifierOp = EGameplayModOp::Override;
	PlayerXPModifier.ModifierMagnitude = FScalableFloat(InitPlayerXP);
	Modifiers.Add(PlayerXPModifier);

	FGameplayModifierInfo PlayerGoldModifier;
	PlayerGoldModifier.Attribute = UPCPlayerAttributeSet::GetPlayerGoldAttribute();
	PlayerGoldModifier.ModifierOp = EGameplayModOp::Override;
	PlayerGoldModifier.ModifierMagnitude = FScalableFloat(InitPlayerGold);
	Modifiers.Add(PlayerGoldModifier);

	FGameplayModifierInfo PlayerHPModifier;
	PlayerHPModifier.Attribute = UPCPlayerAttributeSet::GetPlayerHPAttribute();
	PlayerHPModifier.ModifierOp = EGameplayModOp::Override;
	PlayerHPModifier.ModifierMagnitude = FScalableFloat(InitPlayerHP);
	Modifiers.Add(PlayerHPModifier);
}
