// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Shop/PCShopRequestTypes.h"
#include "PCDataAsset_PlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_PlayerInput : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EPCShopRequestTypes, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* BasicInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_SetDestination;
};
