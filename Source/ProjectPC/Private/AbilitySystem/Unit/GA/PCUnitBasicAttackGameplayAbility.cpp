// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"


UPCUnitBasicAttackGameplayAbility::UPCUnitBasicAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Attack_Basic);
}

void UPCUnitBasicAttackGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass || !Unit || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return;

	UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent();
	if (CooldownGameplayEffectClass && ASC)
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
	
	SetMontage();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
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

		constexpr float Safety = 0.9f; // 안전 여유(쿨다운의 90% 시점에서 끝나도록)
		
		const float DesiredEnd = CooldownDuration * Safety;

		if (DesiredEnd > KINDA_SMALL_NUMBER && MontageLength > KINDA_SMALL_NUMBER)
		{
			// 몽타주가 쿨다운보다 길다면
			if (MontageLength > DesiredEnd)
			{
				PlayRate = FMath::Clamp(MontageLength / DesiredEnd, 1.f, 3.0f);
			}
		}
	}

	return PlayRate;
}
