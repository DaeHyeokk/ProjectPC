// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "PCEffectSpec_Damage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_Damage : public UPCEffectSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayAttribute DamageAttribute;
	
	virtual void ApplyEffect(UAbilitySystemComponent* SourceASC, AActor* Context) override;
};
