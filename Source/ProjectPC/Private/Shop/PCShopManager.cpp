// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"


void UPCShopManager::SetupShopSlots(const TArray<FPCShopUnitData>& NewShopSlots)
{
	ShopSlots = NewShopSlots;

	for (const FPCShopUnitData& ShopSlot : NewShopSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitName : %s, UnitCost : %d"), *ShopSlot.UnitName.ToString(), ShopSlot.UnitCost);
	}
}

const TArray<FPCShopUnitData>& UPCShopManager::GetShopSlots()
{
	return ShopSlots;
}

void UPCShopManager::BuyXP()
{
}

void UPCShopManager::BuyUnit()
{
}

void UPCShopManager::SellUnit()
{
}

void UPCShopManager::ShopRefresh()
{
}

void UPCShopManager::ShopLock()
{
}
