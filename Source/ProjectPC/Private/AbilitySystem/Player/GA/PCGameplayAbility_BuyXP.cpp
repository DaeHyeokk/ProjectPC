// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyXP.h"

#include "BaseGameplayTags.h"


UPCGameplayAbility_BuyXP::UPCGameplayAbility_BuyXP()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyXP);
}
