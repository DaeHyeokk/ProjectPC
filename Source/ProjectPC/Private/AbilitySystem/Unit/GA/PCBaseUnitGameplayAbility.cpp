// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

UPCBaseUnitGameplayAbility::UPCBaseUnitGameplayAbility()
{
	NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy   = EGameplayAbilityReplicationPolicy::ReplicateNo;
	bReplicateInputDirectly = false;

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Dead);
}

void UPCBaseUnitGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	Unit = Cast<APCBaseUnitCharacter>(ActorInfo->AvatarActor);
	if (Unit)
	{
		if (const UPCDataAsset_UnitAbilityConfig* ConfigData = Unit->GetUnitAbilityConfigDataAsset())
		{
			if (ConfigData->TryFindAbilityConfigByTag(AbilityTags.Last(), AbilityConfig))
			{
				UPCUnitGERegistrySubsystem* GERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>();
				if (!GERegistrySubsystem)
					return;
				
				if (AbilityConfig.bUseCost)
				{
					CostGameplayEffectClass = GERegistrySubsystem->GetGEClass(AbilityConfig.CostEffectClassTag);
				}

				if (AbilityConfig.bUseCooldown)
				{
					CooldownGameplayEffectClass = GERegistrySubsystem->GetGEClass(AbilityConfig.CooldownEffectClassTag);
				}
			}
		}
		
		if (Unit && Unit->GetUnitAnimSetDataAsset())
			SetMontageConfig(ActorInfo);
	}
	
	
}

void UPCBaseUnitGameplayAbility::SetMontageConfig(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (const UPCDataAsset_UnitAnimSet* UnitAnimSet = Unit ? Unit->GetUnitAnimSetDataAsset() : nullptr)
	{
		const FGameplayTag MontageTag = GetMontageTag();
		MontageConfig = UnitAnimSet->GetMontageConfigByTag(MontageTag);
	}
}

// void UPCBaseUnitGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
// 	const FGameplayAbilitySpec& Spec)
// {
// 	Super::OnAvatarSet(ActorInfo, Spec);
//
// 	const APCBaseUnitCharacter* Avatar = Cast<APCBaseUnitCharacter>(ActorInfo->AvatarActor.Get());
// 	const UPCDataAsset_UnitAbilityConfig* AbilityConfigData = Avatar ? Avatar->GetUnitAbilityConfigDataAsset() : nullptr;
// 	if (!AbilityConfigData)
// 		return;
//
// 	FAbilityConfig AbilityConfig;
// 	if (AbilityConfigData->TryFindAbilityConfigByTag(AbilityTags.First(), AbilityConfig))
// 	{
// 		CostGameplayEffectClass = AbilityConfig.CostGEClass.LoadSynchronous();
// 		CostEffectCallerTag = AbilityConfig.CostCallerTag;
// 		CostGameplayAttribute = AbilityConfig.CostGameplayAttribute;
//
// 		CooldownGameplayEffectClass = AbilityConfig.CooldownGEClass.LoadSynchronous();
// 		CooldownEffectCallerTag = AbilityConfig.CooldownCallerTag;
// 	}
// }
