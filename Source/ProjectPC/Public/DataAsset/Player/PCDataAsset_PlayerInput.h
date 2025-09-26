// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_PlayerInput.generated.h"

class UInputAction;
class UInputMappingContext;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_PlayerInput : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BasicInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SetDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> BuyXP;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ShopRefresh;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SellUnit;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UNiagaraSystem> FXCursor;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    float ShortPressThreshold;

	// Drag&Drop
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LeftMouse;
	
};
