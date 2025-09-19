// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
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
	
	void SetMontageConfig(const FGameplayAbilityActorInfo* ActorInfo);
	virtual FGameplayTag GetMontageTag() { return FGameplayTag::EmptyTag; }
	
	UPROPERTY(Transient)
	TObjectPtr<APCBaseUnitCharacter> Unit;

	FAbilityConfig AbilityConfig;
	FMontageConfig MontageConfig;
};
