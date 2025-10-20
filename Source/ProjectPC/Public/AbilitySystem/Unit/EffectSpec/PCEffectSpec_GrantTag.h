// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "PCEffectSpec_GrantTag.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_GrantTag : public UPCEffectSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	FGameplayTag GrantTag;

protected:
	virtual FActiveGameplayEffectHandle ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;
	
	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> CachedGEClass;

	TSubclassOf<UGameplayEffect> ResolveGEClass(const UWorld* World);

private:
	FGameplayTag InfiniteGrantTagClassTag = GameplayEffectTags::GE_Class_Unit_GrantTag_Infinite;
	FGameplayTag DurationGrantTagClassTag = GameplayEffectTags::GE_Class_Unit_GrantTag_Duration;
};
