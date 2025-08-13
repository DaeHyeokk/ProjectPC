// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PCCombatGameState.generated.h"

enum class EPCShopRequestTypes : uint8;

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
	
	void ShopRequest(EPCShopRequestTypes RequestType, uint8& PlayerLevel, int32& PlayerGold, uint8 SlotIndex = 0, FName UnitName = "None", uint8 UnitStarCount = 0);
};
