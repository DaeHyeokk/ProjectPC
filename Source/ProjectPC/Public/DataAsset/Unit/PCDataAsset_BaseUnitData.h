// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_BaseUnitData.generated.h"

class UPCDataAsset_ProjectileData;
class UPCDataAsset_UnitAbilityConfig;
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
	FUnitStaticStatConfig(const FGameplayAttribute& InStatAttribute) : StatAttribute(InStatAttribute), StatValue(0.f) {}
	FUnitStaticStatConfig(const FGameplayAttribute& InStatAttribute, const float InStatValue) : StatAttribute(InStatAttribute), StatValue(InStatValue) {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute StatAttribute;

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
	virtual void FillInitStatMap(int32 Level, TMap<FGameplayAttribute, float>& Out) const;
	
	UFUNCTION()
	virtual void FillStartupAbilities(TArray<TSubclassOf<UGameplayAbility>>& OutAbilities) const;

	UFUNCTION()
	UPCDataAsset_UnitAnimSet* GetAnimSetData() const;

	UFUNCTION()
	UPCDataAsset_UnitAbilityConfig* GetAbilityConfigData() const;

	UFUNCTION()
	UPCDataAsset_ProjectileData* GetProjectileData() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Stats|Static")
	TArray<FUnitStaticStatConfig> UnitStaticStatConfigs;
	
	// 기본 공격 평타 능력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TSubclassOf<UGameplayAbility> BasicAttackAbility;
	
	// 이동 관련 능력 (점프, 대쉬  등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> MovementAbilities;

	// Handle GameplayEvent를 통해 발동되는 능력 (Hit React 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> EventAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability Datas")
	TObjectPtr<UPCDataAsset_UnitAbilityConfig> AbilityConfigData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animations")
	TObjectPtr<UPCDataAsset_UnitAnimSet> AnimSetData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UPCDataAsset_ProjectileData> ProjectileData;
};
