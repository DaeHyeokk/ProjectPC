// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_SetByCaller.h"
#include "PCEffectSpec_BasicAttackManaGain.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_BasicAttackManaGain : public UPCEffectSpec_SetByCaller
{
	GENERATED_BODY()

public:
	UPCEffectSpec_BasicAttackManaGain();
	
protected:
	virtual FActiveGameplayEffectHandle ApplyEffectImpl(
		UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;
};
