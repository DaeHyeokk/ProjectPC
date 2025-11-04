// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitGlobalUltGameplayAbility.h"

#include "Character/Unit/PCBaseUnitCharacter.h"


void UPCUnitGlobalUltGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (APCCombatBoard* CombatBoard = Unit->GetOnCombatBoard())
	{
		CurrentFieldUnits.Reset();
		CombatBoard->GetAllFieldUnits(CurrentFieldUnits);
	}
}

void UPCUnitGlobalUltGameplayAbility::OnAttackSucceed(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	if (!Avatar || !ASC)
		return;
	
	if (!CurrentFieldUnits.IsEmpty())
	{
		for (TWeakObjectPtr<APCBaseUnitCharacter> TargetWeak : CurrentFieldUnits)
		{
			if (TargetWeak.IsValid())
			{
				APCBaseUnitCharacter* Target = TargetWeak.Get();
				if (Target && !Target->IsDead())
				{
					ApplyReceivedEventEffectSpec(ASC, AttackSucceedTag, Target);
				}
			}
		}
	}
	
}
