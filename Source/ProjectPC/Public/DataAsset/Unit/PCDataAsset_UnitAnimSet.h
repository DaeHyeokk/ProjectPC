// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAnimSet.generated.h"

class UBlendSpace1D;

USTRUCT(BlueprintType)
struct FMontageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> Montage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintType, Category="Montage|Style")
	bool bHasRecovery = false;
	
};

USTRUCT(BlueprintType)
struct FUnitLocomotionSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UBlendSpace1D> MovementBS; // Walk 1D BS

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> NonCombatIdle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> CombatIdle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> JumpStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> JumpLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> JumpLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> JumpRecovery;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> StunStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> StunIdle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> Death;
};
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitAnimSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion")
	FUnitLocomotionSet LocomotionSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montages")
	TMap<FGameplayTag, FMontageConfig> MontageConfigByTagMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montages")
	TArray<FMontageConfig> BasicAttackMontageConfigs;
	
	UFUNCTION(BlueprintCallable)
	FMontageConfig GetMontageConfigByTag(const FGameplayTag& MontageTag) const;

	UFUNCTION(BlueprintCallable)
	bool TryGetRandomBasicAttackMontageConfigByTag(FMontageConfig& OutConfig) const;
	
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetMontageByTag(const FGameplayTag& MontageTag) const;
};
