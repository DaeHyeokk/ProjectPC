// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_BaseUnitData.generated.h"

class UPCDataAsset_UnitAnimSet;
class UAbilitySystemComponent;
class UPCUnitAbilitySystemComponent;
class UGameplayEffect;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FUnitStaticStatConfig
{
	GENERATED_BODY()

	FUnitStaticStatConfig() : StatValue(0.f) {}
	FUnitStaticStatConfig(const FGameplayTag InStatTag) : StatTag(InStatTag), StatValue(0.f) {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StatTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StatValue;
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_BaseUnitData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPCDataAsset_BaseUnitData();
	
	UFUNCTION()
	virtual void FillInitStatMap(int32 Level, TMap<FGameplayTag, float>& Out) const;
	
	UFUNCTION()
	virtual void FillStartupAbilities(TArray<TSubclassOf<UGameplayAbility>>& OutAbilities) const;

	UFUNCTION()
	UPCDataAsset_UnitAnimSet* GetAnimData() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Stats|Static")
	TArray<FUnitStaticStatConfig> UnitStaticStatConfigs;
	
	// 기본 공격 평타 능력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;
	
	// 방어/패시브 계열 능력 (예: 피해감소, 마나재생 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	// 이동 관련 능력 (점프, 대쉬  등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> MovementAbilities;

	// 기타 특수 능력 (버프, 디버프, 스턴 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> MiscAbilities;

	// Handle GameplayEvent를 통해 발동되는 능력 (Hit React 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> EventAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animations")
	TObjectPtr<UPCDataAsset_UnitAnimSet> AnimData;
};
