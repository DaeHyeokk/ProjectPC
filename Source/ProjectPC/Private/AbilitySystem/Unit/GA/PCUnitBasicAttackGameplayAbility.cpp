// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionJumpForce.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_GrantTag.h"


UPCUnitBasicAttackGameplayAbility::UPCUnitBasicAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Attack_Basic);
}

void UPCUnitBasicAttackGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass || !Unit || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return;
	
	if (UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent())
	{
		const UPCUnitAttributeSet* UnitAttrSet = ASC->GetSet<UPCUnitAttributeSet>();
		const float AttackSpeed = UnitAttrSet->GetEffectiveAttackSpeed();
		const float CooldownDuration = 1.f / FMath::Max(AttackSpeed, 0.0001f);
		
		FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
		if (CooldownSpec.IsValid())
		{
			const FGameplayTag CooldownEffectCallerTag = AbilityConfig.CooldownCallerTag;
			
			CooldownSpec.Data->SetSetByCallerMagnitude(CooldownEffectCallerTag, CooldownDuration);
			ASC->ApplyGameplayEffectSpecToSelf(*CooldownSpec.Data.Get());
		}
	}
}

void UPCUnitBasicAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	bAttackSucceed = false;
	
	SetMontage();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!bAttackSucceed && !AbilityConfig.bSpawnProjectile && GetMontagePlayRate() >= 2.f)
	{
		FGameplayEventData Payload;
		Payload.EventTag = UnitGameplayTags::Unit_Event_AttackSucceed;
		OnAttackSucceed(Payload);

		bAttackSucceed = true;
	}
}

void UPCUnitBasicAttackGameplayAbility::SetMontage()
{
	if (const UPCDataAsset_UnitAnimSet* UnitAnimSet = Unit ? Unit->GetUnitAnimSetDataAsset() : nullptr)
	{
		Montage = UnitAnimSet->GetRandomBasicAttackMontage();
	}
	else
	{
		Montage = nullptr;
	}
}

float UPCUnitBasicAttackGameplayAbility::GetMontagePlayRate()
{
	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	float PlayRate = 1.f;
	
	if (CooldownGameplayEffectClass && ASC && Montage)
	{
		const UPCUnitAttributeSet* UnitAttrSet = ASC->GetSet<UPCUnitAttributeSet>();
		const float AttackSpeed = UnitAttrSet ? UnitAttrSet->GetEffectiveAttackSpeed() : 0.f;
		const float CooldownDuration = 1.f / FMath::Max(AttackSpeed, 0.0001f);
		const float MontageLength = Montage->GetPlayLength();

		// 쿨다운의 90% 시점에서 끝나도록 설정
		constexpr float Safety = 0.9f;
		
		const float DesiredEnd = CooldownDuration * Safety;

		if (DesiredEnd > KINDA_SMALL_NUMBER && MontageLength > KINDA_SMALL_NUMBER)
		{
			// 몽타주가 쿨다운보다 길다면
			if (MontageLength > DesiredEnd)
			{
				PlayRate = FMath::Max(1.f, MontageLength / DesiredEnd);
			}
		}
	}
	return PlayRate;
}

void UPCUnitBasicAttackGameplayAbility::OnAttackSucceed(FGameplayEventData Payload)
{
	if (!bAttackSucceed)
	{
		Super::OnAttackSucceed(Payload);
		bAttackSucceed = true;
	}
}