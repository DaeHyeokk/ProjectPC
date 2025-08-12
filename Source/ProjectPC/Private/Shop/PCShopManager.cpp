// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"


bool UPCShopManager::GetUnitDataByRowName(FName RowName, FPCShopUnitData& OutUnitData)
{
	if (!ShopUnitDataTable) return false;

	// DataTable에서 해당 이름의 행 검색
	if (FPCShopUnitData* FoundData = ShopUnitDataTable->FindRow<FPCShopUnitData>(RowName, TEXT("UPCShopManager::GetUnitDataByUnitName Called")))
	{
		OutUnitData = *FoundData;
		return true;
	}

	return false;
}
