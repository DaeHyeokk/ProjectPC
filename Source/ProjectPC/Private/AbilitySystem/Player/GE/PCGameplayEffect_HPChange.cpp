// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_HPChange.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_HPChange::UPCGameplayEffect_HPChange()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo HPModifier;
	HPModifier.Attribute = UPCPlayerAttributeSet::GetPlayerXPAttribute();
	HPModifier.ModifierOp = EGameplayModOp::Additive;

	Modifiers.Add(HPModifier);
}
