// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"


bool UPCShopManager::GetUnitDataByRowName(FName RowName, FPCShopUnitData& OutUnitData)
{
	if (!ShopUnitDataTable) return false;

	// DataTable에서 해당 이름의 행 검색
	if (FPCShopUnitData* FoundData = ShopUnitDataTable->FindRow<FPCShopUnitData>(RowName, TEXT("UPCShopManager::GetUnitDataByRowName Called")))
	{
		OutUnitData = *FoundData;
		return true;
	}

	return false;
}

bool UPCShopManager::GetUnitProbabilityDataByRowName(FName RowName, FPCShopUnitProbabilityData& OutUnitData)
{
	if (!ShopUnitProbabilityDataTable) return false;

	// DataTable에서 해당 이름의 행 검색
	if (FPCShopUnitProbabilityData* FoundData = ShopUnitProbabilityDataTable->FindRow<FPCShopUnitProbabilityData>(RowName, TEXT("UPCShopManager::GetUnitProbabilityDataByRowName Called")))
	{
		OutUnitData = *FoundData;
		return true;
	}

	return false;
}

void UPCShopManager::UpdateShopSlots(uint8 PlayerLevel)
{
}

TArray<FPCShopUnitData> UPCShopManager::GetShopSlots()
{
	return ShopSlots;
}

bool UPCShopManager::CanBuyUnit(int32 PlayerGold)
{
	return false;
}

bool UPCShopManager::CanBuyXP(int32 PlayerGold)
{
	return false;
}

bool UPCShopManager::CanReroll(int32 PlayerGold)
{
	return false;
}

void UPCShopManager::BuyUnit(uint8 SlotIndex, int32& PlayerGold)
{
}


void UPCShopManager::BuyXP(uint8& PlayerLevel, int32& PlayerGold)
{
}

void UPCShopManager::Reroll(int32& PlayerGold)
{
}

void UPCShopManager::SellUnit(FName UnitName, uint8 UnitStarCount, int32& PlayerGold)
{
}
