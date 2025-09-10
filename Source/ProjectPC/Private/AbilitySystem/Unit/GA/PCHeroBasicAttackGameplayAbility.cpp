// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCHeroBasicAttackGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"


UPCHeroBasicAttackGameplayAbility::UPCHeroBasicAttackGameplayAbility()
{
}

void UPCHeroBasicAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		
	}
}