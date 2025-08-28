// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyUnit.h"

#include "BaseGameplayTags.h"


UPCGameplayAbility_BuyUnit::UPCGameplayAbility_BuyUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyUnit);
}
