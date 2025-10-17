// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "PCBaseUnitGameplayAbility.generated.h"

class APCBaseUnitCharacter;
class UPCUnitAttributeSet;
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

	virtual void OnGiveAbility(
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual TArray<FActiveGameplayEffectHandle> ApplyActivatedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target = nullptr);
	virtual TArray<FActiveGameplayEffectHandle> ApplyCommittedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target = nullptr);
	virtual TArray<FActiveGameplayEffectHandle> ApplyReceivedEventEffectSpec(UAbilitySystemComponent* ASC, const FGameplayTag& ReceivedEventTag, const AActor* Target = nullptr);

	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectSpec(const FPCEffectSpecList* EffectSpecList, UAbilitySystemComponent* ASC, const AActor* Target);
	
	UPROPERTY(Transient)
	TObjectPtr<APCBaseUnitCharacter> Unit;

	FAbilityConfig AbilityConfig;
};
