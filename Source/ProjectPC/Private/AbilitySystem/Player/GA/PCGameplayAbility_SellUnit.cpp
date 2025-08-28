// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_SellUnit.h"

#include "BaseGameplayTags.h"


UPCGameplayAbility_SellUnit::UPCGameplayAbility_SellUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_SellUnit);
}
