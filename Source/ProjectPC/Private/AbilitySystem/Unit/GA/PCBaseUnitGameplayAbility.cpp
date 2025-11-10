// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
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

	if (Unit && Spec.SourceObject.IsValid())
	{
		if (const UPCDataAsset_UnitAbilityConfig* ConfigData = Cast<UPCDataAsset_UnitAbilityConfig>(Spec.SourceObject.Get()))
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
	}
}

void UPCBaseUnitGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!Unit)
	{
		Unit = Cast<APCBaseUnitCharacter>(ActorInfo->AvatarActor);
	}
}

TArray<FActiveGameplayEffectHandle> UPCBaseUnitGameplayAbility::ApplyActivatedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;
	
	if (!HasAuthority(&CurrentActivationInfo) || !ASC)
		return ApplyEffectHandles;
	
	const FPCEffectSpecList* List = &AbilityConfig.OnActivatedEffectSpecs;
	
	ApplyEffectHandles = ApplyEffectSpec(List, ASC, Target);

	return ApplyEffectHandles;
}

TArray<FActiveGameplayEffectHandle> UPCBaseUnitGameplayAbility::ApplyCommittedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;
	
	if (!HasAuthority(&CurrentActivationInfo) || !ASC)
		return ApplyEffectHandles;
	
	const FPCEffectSpecList* List = &AbilityConfig.OnCommittedEffectSpecs;
	
	ApplyEffectHandles = ApplyEffectSpec(List, ASC, Target);

	return ApplyEffectHandles;
}

TArray<FActiveGameplayEffectHandle> UPCBaseUnitGameplayAbility::ApplyReceivedEventEffectSpec(UAbilitySystemComponent* ASC,
	const FGameplayTag& ReceivedEventTag, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;
	
	if (!HasAuthority(&CurrentActivationInfo) || !ASC || !ReceivedEventTag.IsValid())
		return ApplyEffectHandles;

	const FPCEffectSpecList* List = AbilityConfig.OnReceivedEventEffectsMap.Find(ReceivedEventTag);

	ApplyEffectHandles = ApplyEffectSpec(List, ASC, Target);

	return ApplyEffectHandles;
}

TArray<FActiveGameplayEffectHandle> UPCBaseUnitGameplayAbility::ApplyEffectSpec(const FPCEffectSpecList* EffectSpecList,
	UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;

	if (EffectSpecList)
	{
		for (auto& Spec : EffectSpecList->EffectSpecs)
		{
			if (Spec)
			{
				FActiveGameplayEffectHandle EffectHandle;
			
				if (Spec->TargetGroup == EEffectTargetGroup::Self)
				{
					EffectHandle = Spec->ApplyEffectSelf(ASC, GetAbilityLevel());
					if (EffectHandle.IsValid())
						ApplyEffectHandles.Add(EffectHandle);
				}
				else
				{
					if (Spec->TargetGroup == EEffectTargetGroup::All)
					{
						EffectHandle = Spec->ApplyEffectSelf(ASC, GetAbilityLevel());
						if (EffectHandle.IsValid())
							ApplyEffectHandles.Add(EffectHandle);
					}

					if (Target)
					{
						EffectHandle = Spec->ApplyEffect(ASC, Target, GetAbilityLevel());
						if (EffectHandle.IsValid())
							ApplyEffectHandles.Add(EffectHandle);
					}
				}
			}
		}
	}
	return ApplyEffectHandles;
}
