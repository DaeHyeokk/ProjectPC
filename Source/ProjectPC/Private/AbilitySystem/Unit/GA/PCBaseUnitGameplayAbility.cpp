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

	if (const APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(ActorInfo->AvatarActor))
	{
		if (const UPCDataAsset_UnitAbilityConfig* ConfigData = Unit->GetUnitAbilityConfigDataAsset())
		{
			FAbilityConfig AbilityConfig;
			if (ConfigData->TryFindAbilityConfigByTag(AbilityTags.Last(), AbilityConfig))
			{
				UPCUnitGERegistrySubsystem* GERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>();
				if (!GERegistrySubsystem)
					return;
				
				if (AbilityConfig.bUseCost)
				{
					CostGameplayEffectClass = GERegistrySubsystem->GetGEClass(AbilityConfig.CostGETag);
					CostCallerTag = AbilityConfig.CostCallerTag;
					CostGameplayAttribute = AbilityConfig.CostGameplayAttribute;
				}

				if (AbilityConfig.bUseCooldown)
				{
					CooldownGameplayEffectClass = GERegistrySubsystem->GetGEClass(AbilityConfig.CooldownGETag);
					CooldownCallerTag = AbilityConfig.CooldownCallerTag;
				}
			}
		}
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
