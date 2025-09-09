// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"

UPCBaseUnitGameplayAbility::UPCBaseUnitGameplayAbility()
{
	NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy   = EGameplayAbilityReplicationPolicy::ReplicateNo;
	bReplicateInputDirectly = false;
	
}

bool UPCBaseUnitGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return false;

	// Cost Gameplay Effect Class가 None일 경우 True
	if (!CostGameplayEffectClass)
		return true;

	const UPCUnitAttributeSet* UnitAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCUnitAttributeSet>();
	if (!UnitAttributeSet)
		return false;
	
	const float CurrentValue = CostGameplayAttribute.GetGameplayAttributeData(UnitAttributeSet)->GetCurrentValue();
	return CurrentValue >= CostValue;
}

void UPCBaseUnitGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CostGameplayEffectClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	FGameplayEffectSpecHandle CostSpec = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpec.IsValid())
	{
		CostSpec.Data->SetSetByCallerMagnitude(CostEffectCallerTag, -CostValue);
		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpec);
	}
}

void UPCBaseUnitGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CooldownGameplayEffectClass)
	{
		FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
		if (CooldownSpec.IsValid())
		{
			CooldownSpec.Data->SetSetByCallerMagnitude(CooldownEffectCallerTag, CooldownDuration);
			(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
		}
	}
}