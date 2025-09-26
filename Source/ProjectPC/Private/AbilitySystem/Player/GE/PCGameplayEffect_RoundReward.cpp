// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GE/PCGameplayEffect_RoundReward.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayEffect_RoundReward::UPCGameplayEffect_RoundReward()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo XPModifier;
	XPModifier.Attribute = UPCPlayerAttributeSet::GetPlayerXPAttribute();
	XPModifier.ModifierOp = EGameplayModOp::Additive;
	Modifiers.Add(XPModifier);
	
	FGameplayEffectExecutionDefinition GoldExec;
	Executions.Add(GoldExec);
}
