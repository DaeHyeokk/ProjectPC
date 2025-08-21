// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCDataAsset_HeroUnitData.generated.h"

USTRUCT(BlueprintType)
struct FHeroScalableStatConfig
{
	GENERATED_BODY()

	FHeroScalableStatConfig() : StatValue(0.f) {}
	FHeroScalableStatConfig(const FGameplayTag InStatTag) : StatTag(InStatTag), StatValue(0.f) {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StatTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FScalableFloat StatValue;
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_HeroUnitData : public UPCDataAsset_BaseUnitData
{
	GENERATED_BODY()

public:
	UPCDataAsset_HeroUnitData();
	virtual void FillInitStatMap(int32 Level, TMap<FGameplayTag, float>& Out) const override;
	void FillStartupUltimateAbilities(TArray<TSubclassOf<UGameplayAbility>>& OutAbilities) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Stats|Scalable")
	TArray<FHeroScalableStatConfig> HeroScalableStatConfigs;
	
	UPROPERTY(EditDefaultsOnly, Category="Stats|Static")
	TArray<FUnitStaticStatConfig> HeroStaticStatConfigs;

	// 마나가 가득 찼을 때 사용하는 궁극기 능력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Startup|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> UltimateAttackAbilities;
};
