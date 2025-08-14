// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
#include "Shop/PCShopUnitProbabilityData.h"
#include "PCShopManager.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCShopManager : public UActorComponent
{
	GENERATED_BODY()

private:
	TArray<FPCShopUnitData> ShopSlots;
	
public:
	void UpdateShopSlots(uint8 PlayerLevel);
	void BuyUnit(uint8 SlotIndex, int32& PlayerGold);
	void BuyXP(uint8& PlayerLevel, int32& PlayerGold);
	void Reroll(int32& PlayerGold);
	void SellUnit(FName UnitName, uint8 UnitStarCount, int32& PlayerGold);
};
