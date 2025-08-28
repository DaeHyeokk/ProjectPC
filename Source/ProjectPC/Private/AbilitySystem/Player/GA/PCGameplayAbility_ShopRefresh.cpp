// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_ShopRefresh.h"

#include "BaseGameplayTags.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_ShopRefresh::UPCGameplayAbility_ShopRefresh()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_ShopRefresh);
}

void UPCGameplayAbility_ShopRefresh::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Active Ability"));
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	GS->GetShopManager()->UpdateShopSlots();
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_ShopRefresh::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
