// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_ShopLock.h"

#include "BaseGameplayTags.h"


UPCGameplayAbility_ShopLock::UPCGameplayAbility_ShopLock()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_ShopLock);
}
