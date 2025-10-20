// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitStatIcon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitStatIcon : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "StatIcons")
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> StatIcons;
	
	TSoftObjectPtr<UTexture2D> GetStatIcon(FGameplayTag StatTag);
};
