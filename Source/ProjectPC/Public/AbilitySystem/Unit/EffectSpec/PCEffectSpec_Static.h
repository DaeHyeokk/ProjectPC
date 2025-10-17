// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "PCEffectSpec_Static.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_Static : public UPCEffectSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TSubclassOf<UGameplayEffect> GEClass = nullptr;
	
protected:
	virtual FActiveGameplayEffectHandle ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;
	
	
	
};
