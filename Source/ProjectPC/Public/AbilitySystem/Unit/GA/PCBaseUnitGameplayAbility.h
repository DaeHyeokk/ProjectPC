// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PCBaseUnitGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCBaseUnitGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPCBaseUnitGameplayAbility();

protected:
	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Costs")
	FGameplayAttribute CostGameplayAttribute;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Costs")
	FGameplayTag CostCallerTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cooldown")
	FGameplayTag CooldownCallerTag;
};
