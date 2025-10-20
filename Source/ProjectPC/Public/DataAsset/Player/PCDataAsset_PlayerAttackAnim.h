// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_PlayerAttackAnim.generated.h"

USTRUCT(BlueprintType)
struct FAnimationFindKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AttackTypeTag;

	bool operator==(const FAnimationFindKey& Other) const
	{
		return CharacterTag == Other.CharacterTag && AttackTypeTag == Other.AttackTypeTag;
	}
};

FORCEINLINE uint32 GetTypeHash(const FAnimationFindKey& Key)
{
	return HashCombine(GetTypeHash(Key.CharacterTag), GetTypeHash(Key.AttackTypeTag));
}

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_PlayerAttackAnim : public UDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TMap<FAnimationFindKey, TSoftObjectPtr<UAnimMontage>> AttackMontages;

public:
	TSoftObjectPtr<UAnimMontage> GetAttackMontage(FGameplayTag CharacterTag, FGameplayTag AttackTypeTag);
};
