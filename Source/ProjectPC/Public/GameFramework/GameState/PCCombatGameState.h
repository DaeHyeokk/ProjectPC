// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PCCombatGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APCCombatGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShopManager")
	class UPCShopManager* ShopManager;
	
	
};
