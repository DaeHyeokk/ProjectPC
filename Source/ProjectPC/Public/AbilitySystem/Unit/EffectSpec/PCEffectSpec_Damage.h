// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "BaseGameplayTags.h"
#include "PCEffectSpec_SetByCaller.h"
#include "PCEffectSpec_Damage.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Basic		UMETA(DisplayName="Basic Attack"),
	Ultimate	UMETA(DisplayName="Ultimate"),
	BonusDamage UMETA(DisplayName="Bonus Damage"),
	None		UMETA(DisplayName="None"),
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCEffectSpec_Damage : public UPCEffectSpec_SetByCaller
{
	GENERATED_BODY()

public:
	UPCEffectSpec_Damage();
	
protected:
	virtual FActiveGameplayEffectHandle ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Unit.AttackType"))
	FGameplayTag AttackType = UnitGameplayTags::Unit_AttackType_Basic;
		
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Unit.DamageType"))
	FGameplayTag DamageType = UnitGameplayTags::Unit_DamageType_Physical;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(InlineEditConditionToggle))
	bool bUseDamageAttribute = true;
	
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(EditCondition="bUseDamageAttribute"))
	FGameplayAttribute DamageAttribute;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(InlineEditConditionToggle))
	bool bUseFixedDamage = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(EditCondition="bUseFixedDamage"))
	float FixedDamage = 0.f;

	UPROPERTY(Transient)
	bool bUseDamageOverride = false;
	
	UPROPERTY(Transient)
	float DamageOverride = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage|Flag")
	bool bNoCrit = false;

	UPROPERTY(EditDefaultsOnly, Category="Damage|Flag")
	bool bNoVamp = false;

	UPROPERTY(EditDefaultsOnly, Category="Damage|Flag")
	bool bNoManaGain = false;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	bool bNoSendHitEvent = false;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	bool bNoSendAppliedDamageEvent = false;
	
	FGameplayTag BasicHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_Basic;
	FGameplayTag UltimateHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_Ultimate;
	FGameplayTag BonusDmgHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_BonusDamage;
	FGameplayTag OnHitEventTag = UnitGameplayTags::Unit_Event_OnHit;

	FGameplayTag NoCritTag = UnitGameplayTags::Unit_DamageFlag_NoCrit;
	FGameplayTag NoVampTag = UnitGameplayTags::Unit_DamageFlag_NoVamp;
	FGameplayTag NoManaGainTag = UnitGameplayTags::Unit_DamageFlag_NoManaGain;
	FGameplayTag NoSendHitEventTag = UnitGameplayTags::Unit_DamageFlag_NoSendHitEvent;
	FGameplayTag NoSendDamageAppliedEventTag = UnitGameplayTags::Unit_DamageFlag_NoSendDamageAppliedEvent;

public:
	FORCEINLINE void SetDamage(const float InDamage)
	{
		DamageOverride = InDamage;
		bUseDamageOverride = true;
	}
};
