// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"


UPCUnitBasicAttackGameplayAbility::UPCUnitBasicAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Action_Attack_Basic);

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_Action_Attack_Basic);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Action_Attack_Basic);

	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_Action_Attack_Basic);
}

void UPCUnitBasicAttackGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	UPCUnitGERegistrySubsystem* UnitGERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>();
	if (!UnitGERegistrySubsystem)
		return;

	const FGameplayTag CooldownEffectTag = GameplayEffectTags::GE_Class_Cooldown_BasicAttack;
	CooldownGameplayEffectClass = UnitGERegistrySubsystem->GetGEClass(CooldownEffectTag);

	const FGameplayTag ManaGainEffectTag = GameplayEffectTags::GE_Class_Mana_Gain_Instant;
	ManaGainEffectClass = UnitGERegistrySubsystem->GetGEClass(ManaGainEffectTag);
}

void UPCUnitBasicAttackGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass || !UnitAttrSet || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return;
	
	if (CooldownGameplayEffectClass && UnitAttrSet)
	{
		const float AttackSpeed = UnitAttrSet->GetAttackSpeed();
		const float CooldownDuration = 1.f / FMath::Max(AttackSpeed, 0.0001f);
		
		FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
		if (CooldownSpec.IsValid())
		{
			const FGameplayTag CooldownEffectCallerTag = GameplayEffectTags::GE_Caller_Cooldown_BasicAttack;
			
			CooldownSpec.Data->SetSetByCallerMagnitude(CooldownEffectCallerTag, CooldownDuration);
			(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
		}
	}
}

UAnimMontage* UPCUnitBasicAttackGameplayAbility::GetMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FGameplayTag BasicAttackMontageTag = UnitGameplayTags::Unit_Montage_Attack_Basic;
	return UnitAnimSet->GetAnimMontageByTag(BasicAttackMontageTag);
}

float UPCUnitBasicAttackGameplayAbility::GetMontagePlayRate(const UAnimMontage* Montage)
{
	float PlayRate = 1.f;
	
	if (CooldownGameplayEffectClass && UnitAttrSet)
	{
		const float AttackSpeed = UnitAttrSet->GetAttackSpeed();
		const float CooldownDuration = 1.f / FMath::Max(AttackSpeed, 0.0001f);
		const float MontageLength = Montage->GetPlayLength();

		constexpr float Safety = 0.9f; // 안전 여유(쿨다운의 90% 시점에서 끝나도록)
		
		const float DesiredEnd = CooldownDuration * Safety;

		if (DesiredEnd > KINDA_SMALL_NUMBER && MontageLength > KINDA_SMALL_NUMBER)
		{
			// 몽타주가 쿨다운보다 길다면
			if (MontageLength > DesiredEnd)
			{
				PlayRate = FMath::Clamp(MontageLength / DesiredEnd, 0.2f, 3.0f);
			}
		}
	}

	return PlayRate;
}

void UPCUnitBasicAttackGameplayAbility::ApplyGameplayEffect()
{
	if (ManaGainEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ManaGainEffectClass, 1.f);
		if (SpecHandle.IsValid())
		{
			const FGameplayTag CallerTag = GameplayEffectTags::GE_Caller_ManaChange;
			SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, ManaGainAmount);

			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
