// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "PCUnitMontagePlayGameplayAbility.h"
#include "PCUnitBaseAttackGameplayAbility.generated.h"

class UPCUnitAbilitySystemComponent;
class UPCDataAsset_UnitAnimSet;
class UPCUnitAttributeSet;
/**
 * 
 */
UCLASS(Abstract)
class PROJECTPC_API UPCUnitBaseAttackGameplayAbility : public UPCUnitMontagePlayGameplayAbility
{
	GENERATED_BODY()

public:
	UPCUnitBaseAttackGameplayAbility();
	
protected:
	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	void SetCurrentTarget(const AActor* Avatar);
	
	UPROPERTY(Transient)
	TWeakObjectPtr<const AActor> CurrentTarget;

	const FGameplayTag SpawnProjectileSucceedTag = UnitGameplayTags::Unit_Event_SpawnProjectileSucceed;
	const FGameplayTag AttackSucceedTag = UnitGameplayTags::Unit_Event_AttackSucceed;
	
protected:
	void StartAttackSucceedWaitTask();
	void StartProjectileSpawnSucceedWaitTask();
	
	UFUNCTION()
	virtual void AttackCommit();
	
	UFUNCTION()
	void OnAttackSucceed(FGameplayEventData Payload);

	UFUNCTION()
	void OnSpawnProjectileSucceed(FGameplayEventData Payload);
};

