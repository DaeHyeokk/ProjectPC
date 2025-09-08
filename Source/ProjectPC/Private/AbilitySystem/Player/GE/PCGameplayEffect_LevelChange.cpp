// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_LevelChange.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_LevelChange::UPCGameplayEffect_LevelChange()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo LevelModifier;
	LevelModifier.Attribute = UPCPlayerAttributeSet::GetPlayerXPAttribute();
	LevelModifier.ModifierOp = EGameplayModOp::Additive;

	Modifiers.Add(LevelModifier);
}
