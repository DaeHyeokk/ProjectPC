// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/Synergy/PCSynergyMechanicGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"


UPCSynergyMechanicGameplayAbility::UPCSynergyMechanicGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Synergy_Mechanic);
	WaitEventTag = UnitGameplayTags::Unit_Event_DamageApplied;
}

TArray<FActiveGameplayEffectHandle> UPCSynergyMechanicGameplayAbility::ApplyCommittedEffectSpec(
	UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;
	
	if (!HasAuthority(&CurrentActivationInfo) || !ASC)
		return ApplyEffectHandles;
	
	const FPCEffectSpecList* List = &AbilityConfig.OnCommittedEffectSpecs;
	
	// Damage Effect를 찾은다음 데미지 세팅
	if (const FPCEffectSpecList* EffectSpecList = &AbilityConfig.OnCommittedEffectSpecs)
	{
		for (UPCEffectSpec* EffectSpec : EffectSpecList->EffectSpecs)
		{
			if (UPCEffectSpec_Damage* DamageEffect = Cast<UPCEffectSpec_Damage>(EffectSpec))
			{
				if (const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
				{
					const float CurrentHP = TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetCurrentHealthAttribute());
					DamageEffect->SetDamage(CurrentHP);
				}
				break;
			}
		}
	}
	
	ApplyEffectHandles = ApplyEffectSpec(List, ASC, Target);

	return ApplyEffectHandles;
}

bool UPCSynergyMechanicGameplayAbility::ShouldCommitOnEvent_Implementation(const FGameplayEventData& Payload)
{
	const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target);
	if (!TargetASC)
		return false;

	const float CurrentHP = TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetCurrentHealthAttribute());
	const float MaxHP = FMath::Max(1.f, TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetMaxHealthAttribute()));
	const float Percent = (CurrentHP / MaxHP) * 100.f;
	const float ExecutionHpPercent = ExecutionHpThresholdPercent.GetValueAtLevel(GetAbilityLevel());
	
	return Percent <= ExecutionHpPercent;
}
