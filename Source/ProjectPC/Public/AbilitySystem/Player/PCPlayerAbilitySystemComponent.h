// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "PCPlayerAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset")
	class UPCDataAsset_PlayerAbilities* PlayerAbilityData;

public:
	// Initial GE 적용
	void ApplyInitializedEffects();
	// Initial GA 적용
	void ApplyInitializedAbilities();
	// Tag에 따른 GE 적용
	void ApplyPlayerEffects(FGameplayTag GE_Tag, float GE_Value);
};
