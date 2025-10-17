// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "PCUnitMontagePlayGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitMontagePlayGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitMontagePlayGameplayAbility();
	
protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	FMontageConfig MontageConfig;

	virtual void SetMontageConfig();
	virtual FGameplayTag GetMontageTag() { return FGameplayTag::EmptyTag; }
	virtual float GetMontagePlayRate(const UAnimMontage* Montage) { return 1.f; }
	
	virtual void StartPlayMontageAndWaitTask(UAnimMontage* Montage, const bool bStopWhenAbilityEnds = true);
	
	UFUNCTION()
	virtual void OnMontageFinished();

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
