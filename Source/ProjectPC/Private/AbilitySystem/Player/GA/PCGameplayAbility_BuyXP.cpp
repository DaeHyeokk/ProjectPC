// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyXP.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayAbility_BuyXP::UPCGameplayAbility_BuyXP()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyXP);
}

bool UPCGameplayAbility_BuyXP::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo->IsNetAuthority() || !CostGameplayEffectClass)
	{
		return false;
	}

	if (const auto* CostAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCPlayerAttributeSet>())
	{
		if (static_cast<int32>(CostAttributeSet->GetPlayerLevel()) == 10)
		{
			return false;
		}
		
		return CostAttributeSet->GetPlayerGold() >= CostValue;
	}
	
	return false;
}

void UPCGameplayAbility_BuyXP::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpecHandle.IsValid())
	{
		CostSpecHandle.Data->SetSetByCallerMagnitude(CostTag, -CostValue);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
	}
}

void UPCGameplayAbility_BuyXP::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) 
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle XPSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerXPChange, GetAbilityLevel());
	if (XPSpecHandle.IsValid())
	{
		XPSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerXP, GE_Value);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*XPSpecHandle.Data.Get());
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_BuyXP::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
