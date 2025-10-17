// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "PCEffectSpec.generated.h"

/**
 * 
 */

class UGameplayEffect;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EEffectTargetGroup : uint8
{
	Hostile,
	Ally,
	Self,
	All
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType)
class PROJECTPC_API UPCEffectSpec : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	EEffectTargetGroup TargetGroup = EEffectTargetGroup::Self;

	UPROPERTY(EditDefaultsOnly, Category="Effect|Duration", meta=(InlineEditConditionToggle))
	bool bUseDurationSetByCaller = false;

	UPROPERTY(EditDefaultsOnly, Category="Effect|Duration", meta=(EditCondition="bUseDurationSetByCaller", Categories="GE.Caller"))
	FGameplayTag DurationCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Effect|Duration", meta=(EditCondition="bUseDurationSetByCaller", ClampMin="0.0"))
	float DurationByCallerSeconds = 0.f;
	
	int32 DefaultLevel = 1.f;
	
	FActiveGameplayEffectHandle  ApplyEffect(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel = -1);
	FActiveGameplayEffectHandle ApplyEffectSelf(UAbilitySystemComponent* ASC, int32 EffectLevel = -1);
	
protected:
	virtual FActiveGameplayEffectHandle  ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) PURE_VIRTUAL(UPCEffectSpec::ApplyEffectImpl, return FActiveGameplayEffectHandle();)
	
	bool IsTargetEligibleByGroup(const AActor* Source, const AActor* Target) const;

	void ApplyDurationOptions(FGameplayEffectSpec& Spec) const;
};
