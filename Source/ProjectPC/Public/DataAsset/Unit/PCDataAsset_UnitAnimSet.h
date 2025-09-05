// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAnimSet.generated.h"

class UBlendSpace1D;

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
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> MontageByTagMap;

	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetAnimMontageByTag(const FGameplayTag& MontageTag) const;
};
