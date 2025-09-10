// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

// bool UPCUnitBasicAttackGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
// 	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
// {
// 	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
// 		return false;
//
// 	// Cost Gameplay Effect Class가 None일 경우 True
// 	if (!CostGameplayEffectClass)
// 		return true;
//
// 	const UPCUnitAttributeSet* UnitAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCUnitAttributeSet>();
// 	if (!UnitAttributeSet)
// 		return false;
// 	
// 	const float CurrentValue = CostGameplayAttribute.GetGameplayAttributeData(UnitAttributeSet)->GetCurrentValue();
// 	//return CurrentValue >= CostMagnitude;
// 	return true;
// }
//
// void UPCUnitBasicAttackGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
// 	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
// {
// 	if (!CostGameplayEffectClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
// 	{
// 		return;
// 	}
//
// 	FGameplayEffectSpecHandle CostSpec = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
// 	if (CostSpec.IsValid())
// 	{
// 		CostSpec.Data->SetSetByCallerMagnitude(CostEffectCallerTag, -CostMagnitude);
// 		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpec);
// 	}
// }

void UPCUnitBasicAttackGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilitySpec& Spec)
{
	UPCUnitGERegistrySubsystem* UnitGERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>();
	if (!UnitGERegistrySubsystem)
		return;

	FGameplayTag CooldownEffectTag = GameplayEffectTags::GE_Class_Cooldown_BasicAttack;
	CooldownGameplayEffectClass = UnitGERegistrySubsystem->GetGEClass(CooldownEffectTag);
}

void UPCUnitBasicAttackGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return;

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UPCUnitAttributeSet* UnitAttributeSet = ASC ? ASC->GetSet<UPCUnitAttributeSet>() : nullptr;
	if (!UnitAttributeSet)
		return;

	const float AttackSpeed = UnitAttributeSet->GetAttackSpeed();
	const float CooldownDuration = 1.f / FMath::Max(AttackSpeed, 0.0001f);
	
	if (CooldownGameplayEffectClass)
	{
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
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(ActorInfo->OwnerActor.Get());
	const UPCDataAsset_UnitAnimSet* UnitAnimSet = UnitCharacter ? UnitCharacter->GetUnitAnimSetDataAsset() : nullptr;
	if (!UnitAnimSet)
		return nullptr;

	return UnitAnimSet->GetRandomBasicAttackMontage();
}
