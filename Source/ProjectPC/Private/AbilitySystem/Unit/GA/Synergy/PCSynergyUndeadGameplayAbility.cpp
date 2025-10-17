// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/Synergy/PCSynergyUndeadGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"

UPCSynergyUndeadGameplayAbility::UPCSynergyUndeadGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Synergy_Undead);
	WaitEventTag = UnitGameplayTags::Unit_Event_HitSucceed;
	bOnlyMatchExact = false;
}

void UPCSynergyUndeadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		RevokeAllCachedEffects();
		bAttackSpeedApplied = false;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPCSynergyUndeadGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HasAuthority(&ActivationInfo))
	{
		RevokeAllCachedEffects();
		bAttackSpeedApplied = false;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPCSynergyUndeadGameplayAbility::RevokeAllCachedEffects()
{
	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
		return;

	for (FActiveGameplayEffectHandle& Handle : CachedEffectHandles)
	{
		if (Handle.IsValid())
			ASC->RemoveActiveGameplayEffect(Handle);
	}
	
	CachedEffectHandles.Reset();
}

TArray<FActiveGameplayEffectHandle> UPCSynergyUndeadGameplayAbility::ApplyCommittedEffectSpec(
	UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> EffectHandles = Super::ApplyCommittedEffectSpec(ASC, Target);;

	if (HasAuthority(&CurrentActivationInfo))
	{
		if (ASC)
		{
			for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
			{
				if (Handle.IsValid())
					CachedEffectHandles.Add(Handle);
			}
		}
	}
	
	return EffectHandles;
}

bool UPCSynergyUndeadGameplayAbility::ShouldCommitOnEvent_Implementation(const FGameplayEventData& Payload)
{
	if (bAttackSpeedApplied)
		return false;

	const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target);
	if (!TargetASC)
		return false;

	const float CurrentHP = TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetCurrentHealthAttribute());
	const float MaxHP = FMath::Max(1.f, TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetMaxHealthAttribute()));
	const float Percent = CurrentHP / MaxHP;

	const bool ShouldCommit = (Percent < 0.5f);
	if (ShouldCommit)
		bAttackSpeedApplied = true;

	return ShouldCommit;
}
