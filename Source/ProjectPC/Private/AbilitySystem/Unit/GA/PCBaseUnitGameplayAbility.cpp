// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"

UPCBaseUnitGameplayAbility::UPCBaseUnitGameplayAbility()
{
	NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy   = EGameplayAbilityReplicationPolicy::ReplicateNo;
	bReplicateInputDirectly = false;
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
