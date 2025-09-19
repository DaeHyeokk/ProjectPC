// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "PCEffectSpec_Damage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_Damage : public UPCEffectSpec
{
	GENERATED_BODY()

protected:
	virtual void ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayAttribute DamageAttribute;
	
	FGameplayTag HitEventTag = UnitGameplayTags::Unit_Event_OnHit;
};
