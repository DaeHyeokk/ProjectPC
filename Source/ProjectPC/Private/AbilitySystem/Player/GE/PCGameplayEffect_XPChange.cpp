// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_XPChange.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_XPChange::UPCGameplayEffect_XPChange()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo XPModifier;
	XPModifier.Attribute = UPCPlayerAttributeSet::GetPlayerXPAttribute();
	XPModifier.ModifierOp = EGameplayModOp::Additive;

	Modifiers.Add(XPModifier);
}
