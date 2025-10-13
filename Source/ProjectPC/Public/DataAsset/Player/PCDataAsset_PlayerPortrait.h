// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_PlayerPortrait.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_PlayerPortrait : public UDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> PlayerPortraits;

public:
	TSoftObjectPtr<UTexture2D> GetPlayerPortrait(FGameplayTag CharacterTag);
};
