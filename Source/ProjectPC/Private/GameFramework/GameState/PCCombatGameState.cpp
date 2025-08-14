// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}

void APCCombatGameState::BeginPlay()
{
	Super::BeginPlay();

	if (ShopUnitDataTable == nullptr) return;
	if (ShopUnitProbabilityDataTable == nullptr) return;

	LoadDataTable<FPCShopUnitData>(ShopUnitDataTable, ShopUnitDataList, TEXT("Loading Shop Unit Data"));
	LoadDataTable<FPCShopUnitProbabilityData>(ShopUnitProbabilityDataTable, ShopUnitProbabilityDataList, TEXT("Loading Shop Unit Probability Data"));
}

// void APCCombatGameState::Server_ShopRequest(EPCShopRequestTypes RequestType)
// {
// 	switch (RequestType)
// 	{
// 	case EPCShopRequestTypes::UpdateSlot:
// 		ShopManager->UpdateShopSlots(PlayerLevel);
// 		break;
// 	case EPCShopRequestTypes::BuyUnit:
// 		ShopManager->BuyUnit(SlotIndex, PlayerGold);
// 		break;
// 	case EPCShopRequestTypes::BuyXP:
// 		ShopManager->BuyXP(PlayerLevel, PlayerGold);
// 		break;
// 	case EPCShopRequestTypes::Reroll:
// 		ShopManager->Reroll(PlayerGold);
// 		break;
// 	case EPCShopRequestTypes::SellUnit:
// 		ShopManager->SellUnit(UnitName, UnitStarCount, PlayerGold);
// 		break;
// 	default:
// 		break;
// 	}
// }

TArray<FPCShopUnitData>& APCCombatGameState::GetShopUnitDataList()
{
	return ShopUnitDataList;
}

const TArray<FPCShopUnitProbabilityData>& APCCombatGameState::GetShopUnitProbabilityDataList()
{
	return ShopUnitProbabilityDataList;
}
