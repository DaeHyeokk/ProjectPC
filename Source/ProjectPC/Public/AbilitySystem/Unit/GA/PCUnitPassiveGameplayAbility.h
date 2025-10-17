// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "PCUnitPassiveGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitPassiveGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void OnGiveAbility(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;
	
	UFUNCTION()
	bool RollChance() const;
	float GetChancePercentForLevel(int32 Level) const;

	UPROPERTY(EditDefaultsOnly, Category="Passive|Chance", meta=(InlineEditConditionToggle))
	bool bUseChance = false;

	UPROPERTY(EditDefaultsOnly, Category="Passive|Chance",
		meta=(EditCondition="bUseChance", ClampMin="0.0", ClampMax="100.0", UIMin="0.0", UIMax="100.0"))
	TArray<float> ChancePercentByLevel;
};
