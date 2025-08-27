// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
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
	void SetupShopSlots(const TArray<FPCShopUnitData>& NewShopSlots);
	const TArray<FPCShopUnitData>& GetShopSlots();
	
	void BuyXP();
	void BuyUnit();
	void SellUnit();
	void ShopRefresh();
	void ShopLock();
};
