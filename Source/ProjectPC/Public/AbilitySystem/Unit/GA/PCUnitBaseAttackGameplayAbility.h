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

	UPROPERTY(Transient)
	TWeakObjectPtr<const AActor> CurrentTarget;

	const FGameplayTag AttackCommitEventTag = UnitGameplayTags::Unit_Event_AttackCommit;
	const FGameplayTag HitSucceedTag = UnitGameplayTags::Unit_Event_HitSucceed;
	bool bDidCommit = false;
	
	void StartAttackCommitWaitTask();
	void StartPlayMontageAndWaitTask(UAnimMontage* Montage);
	
protected:
	UFUNCTION()
	virtual void OnAttackCommit(FGameplayEventData Payload);
	UFUNCTION()
	void OnMontageFinished();

	void SpawnProjectileFromConfig();
	void DoMeleeImpactAndApply();
	
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
