// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/Synergy/PCSynergyCyborgGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"


UPCSynergyCyborgGameplayAbility::UPCSynergyCyborgGameplayAbility()
{
	PeriodSeconds = 1.f;
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Synergy_Cyborg);
}

void UPCSynergyCyborgGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	RevokeAllPerStackEffects();
	bIsListening = false;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPCSynergyCyborgGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	StopDamageAppliedEventWaitTask();
	RevokeAllPerStackEffects();
	bIsListening = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

TArray<FActiveGameplayEffectHandle> UPCSynergyCyborgGameplayAbility::ApplyCommittedEffectSpec(
	UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> OutHandles;

	if (!ASC)
		return OutHandles;

	if (CurrentStack >= MaxStacks)
	{
		if (!bIsListening && DamageAppliedEventTag.IsValid())
			StartDamageAppliedEventWaitTask();

		return OutHandles;
	}
	
	OutHandles = Super::ApplyCommittedEffectSpec(ASC, Target);

	for (const FActiveGameplayEffectHandle& Handle : OutHandles)
	{
		if (Handle.IsValid())
			ActivePerStackEffectHandles.Add(Handle);
	}
	
	++CurrentStack;
	
	if (CurrentStack >= MaxStacks && DamageAppliedEventTag.IsValid() && !bIsListening)
		StartDamageAppliedEventWaitTask();

	return OutHandles;
}

void UPCSynergyCyborgGameplayAbility::RevokeAllPerStackEffects()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		for (FActiveGameplayEffectHandle& Handle : ActivePerStackEffectHandles)
		{
			if (Handle.IsValid())
				ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	CurrentStack = 0;
	ActivePerStackEffectHandles.Reset();
}

void UPCSynergyCyborgGameplayAbility::StartDamageAppliedEventWaitTask()
{
	if (!DamageAppliedEventTag.IsValid() || bIsListening)
		return;

	WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DamageAppliedEventTag, nullptr, false);
	if (WaitTask)
	{
		WaitTask->EventReceived.AddDynamic(this, &ThisClass::OnDamageAppliedEventReceived);
		WaitTask->ReadyForActivation();
		bIsListening = true;
	}
}

void UPCSynergyCyborgGameplayAbility::StopDamageAppliedEventWaitTask()
{
	if (WaitTask)
	{
		WaitTask->EndTask();
		WaitTask = nullptr;
	}
	bIsListening = false;
}

void UPCSynergyCyborgGameplayAbility::OnDamageAppliedEventReceived(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const AActor* Target = Payload.Target;
	if (!ASC || !Target)
		return;
	
	const float FinalDamage = Payload.EventMagnitude;
	const float BonusDmgPct = (BonusTrueDamagePercentage.GetValueAtLevel(GetAbilityLevel()) * 0.01f);
	if (FinalDamage <= KINDA_SMALL_NUMBER || BonusDmgPct <= 0.f)
		return;

	const float BonusTrueDmg = FinalDamage * BonusDmgPct;

	// Damage Effect를 찾은다음 데미지 세팅
	if (const FPCEffectSpecList* EffectSpecList = AbilityConfig.OnReceivedEventEffectsMap.Find(DamageAppliedEventTag))
	{
		for (UPCEffectSpec* EffectSpec : EffectSpecList->EffectSpecs)
		{
			if (UPCEffectSpec_Damage* DamageEffect = Cast<UPCEffectSpec_Damage>(EffectSpec))
			{
				DamageEffect->SetDamage(BonusTrueDmg);
				break;
			}
		}
	}
	ApplyReceivedEventEffectSpec(ASC, DamageAppliedEventTag, Target);
}