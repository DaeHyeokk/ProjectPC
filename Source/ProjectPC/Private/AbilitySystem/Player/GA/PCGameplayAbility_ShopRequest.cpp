// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_ShopRequest.h"

#include "AbilitySystemComponent.h"

#include "GameFramework/GameState/PCCombatGameState.h"


void UPCGameplayAbility_ShopRequest::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS || !GS->ShopManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) 
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 어떤 하위 GA를 실행할지 결정
	if (AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Player.GA.Shop.BuyXP")))
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Player.GA.Shop.BuyXP")));
	}
	else if (AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Player.GA.Shop.BuyUnit")))
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Player.GA.Shop.BuyUnit")));
	}
	else if (AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Player.GA.Shop.SellUnit")))
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Player.GA.Shop.SellUnit")));
	}
	else if (AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Player.GA.Shop.ShopRefresh")))
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Player.GA.Shop.ShopRefresh")));
	}
	else if (AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Player.GA.Shop.ShopLock")))
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Player.GA.Shop.ShopLock")));
	}
}

void UPCGameplayAbility_ShopRequest::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
