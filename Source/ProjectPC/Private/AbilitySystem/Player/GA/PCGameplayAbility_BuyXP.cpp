// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyXP.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


UPCGameplayAbility_BuyXP::UPCGameplayAbility_BuyXP()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyXP);

	ActivationRequiredTags.AddTag(PlayerGameplayTags::Player_State_Normal);

	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Dead);
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Carousel);
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_MaxLevel);
}

bool UPCGameplayAbility_BuyXP::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 서버 권위, CostGE 클래스가 유효하면 Activate
	if (ActorInfo && ActorInfo->IsNetAuthority() && CostGameplayEffectClass)
	{
		return true;
	}
	
	return false;
}

bool UPCGameplayAbility_BuyXP::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (const auto AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCPlayerAttributeSet>())
		{
			return AttributeSet->GetPlayerGold() >= CostValue;
		}
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
		
		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
		}
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
	
	FGameplayEffectSpecHandle XPSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerXPChange, GetAbilityLevel());
	if (XPSpecHandle.IsValid())
	{
		XPSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerXP, GE_Value);

		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*XPSpecHandle.Data.Get());
			ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTags::GameplayCue_Player_BuyXP);
		}
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_BuyXP::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
