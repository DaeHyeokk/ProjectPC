// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/Player/PCDataAsset_PlayerAttackAnim.h"
#include "PCGameplayAbility_DamageEvent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameplayAbility_DamageEvent : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCGameplayAbility_DamageEvent();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GE")
	TSubclassOf<UGameplayEffect> GE_PlayerHPChange;

	UPROPERTY(EditDefaultsOnly, Category = "AttackAnim")
	TObjectPtr<UPCDataAsset_PlayerAttackAnim> PlayerAttackAnimData;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};