// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "PCUnitBaseAttackGameplayAbility.generated.h"

class UPCUnitAbilitySystemComponent;
class UPCDataAsset_UnitAnimSet;
class UPCUnitAttributeSet;
/**
 * 
 */
UCLASS(Abstract)
class PROJECTPC_API UPCUnitBaseAttackGameplayAbility : public UPCBaseUnitGameplayAbility
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
	virtual float GetMontagePlayRate(const UAnimMontage* Montage) { return 1.f; }

	virtual void SetMontageConfig(const FGameplayAbilityActorInfo* ActorInfo);
	virtual FGameplayTag GetMontageTag() { return FGameplayTag::EmptyTag; }
	
	UPROPERTY(Transient)
	TWeakObjectPtr<const AActor> CurrentTarget;

	const FGameplayTag SpawnProjectileSucceedTag = UnitGameplayTags::Unit_Event_SpawnProjectileSucceed;
	const FGameplayTag HitSucceedTag = UnitGameplayTags::Unit_Event_HitSucceed;

	FMontageConfig MontageConfig;
	
protected:
	void StartHitSucceedWaitTask();
	void StartProjectileSpawnSucceedWaitTask();
	void StartPlayMontageAndWaitTask(UAnimMontage* Montage);
	
	UFUNCTION()
	void AttackCommit();
	
	UFUNCTION()
	void OnHitSucceed(FGameplayEventData Payload);

	UFUNCTION()
	void OnSpawnProjectileSucceed(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageFinished();
	
protected:
	// ==== 디버깅용 ====
	// UFUNCTION()
	// void OnMontageCompleted();
	// UFUNCTION()
	// void OnMontageCancelled();
	// UFUNCTION()
	// void OnMontageBlendOut();
	// UFUNCTION()
	// void OnMontageInterrupted();
};

