// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_ShopRefresh.h"

#include "AbilitySystemComponent.h"
#include "BaseGameplayTags.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_ShopRefresh::UPCGameplayAbility_ShopRefresh()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_ShopRefresh);
	
	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_GA_Shop_ShopRefresh;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UPCGameplayAbility_ShopRefresh::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo->IsNetAuthority() || !CostGameplayEffectClass)
	{
		return false;
	}
	
	return true;
}

bool UPCGameplayAbility_ShopRefresh::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}

void UPCGameplayAbility_ShopRefresh::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpecHandle.IsValid())
	{
		CostSpecHandle.Data->SetSetByCallerMagnitude(CostTag, -CostValue);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
	}
}

void UPCGameplayAbility_ShopRefresh::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (const auto* CostAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCPlayerAttributeSet>())
	{
		CostValue = TriggerEventData->EventMagnitude;
		if (CostAttributeSet->GetPlayerGold() >= CostValue)
		{
			if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) 
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}

			if (auto GS = GetWorld()->GetGameState<APCCombatGameState>())
			{
				if (auto PS = ActorInfo->PlayerController->GetPlayerState<APCPlayerState>())
				{
					GS->GetShopManager()->UpdateShopSlots(PS);
				}
			}
		}
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_ShopRefresh::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
