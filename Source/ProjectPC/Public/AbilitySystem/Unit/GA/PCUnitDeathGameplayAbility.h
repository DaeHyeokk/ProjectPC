// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "PCUnitDeathGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitDeathGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitDeathGameplayAbility();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//virtual FGameplayTag GetMontageTag() override { return UnitGameplayTags::Unit_Montage_Death; }
};
