// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
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
	TMap<FGameplayTag, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* BasicInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* SetDestination;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UNiagaraSystem* FXCursor;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    float ShortPressThreshold;

	// Drag&Drop
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_LeftMouse;
	
};
