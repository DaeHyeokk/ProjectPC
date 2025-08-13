// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Shop/PCShopManager.h"
#include "Shop/PCShopRequestTypes.h"


APCCombatGameState::APCCombatGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}

void APCCombatGameState::ShopRequest(EPCShopRequestTypes RequestType, uint8& PlayerLevel, int32& PlayerGold, uint8 SlotIndex, FName UnitName, uint8 UnitStarCount)
{
	switch (RequestType)
	{
	case EPCShopRequestTypes::UpdateSlot:
		ShopManager->UpdateShopSlots(PlayerLevel);
		break;
	case EPCShopRequestTypes::BuyUnit:
		ShopManager->BuyUnit(SlotIndex, PlayerGold);
		break;
	case EPCShopRequestTypes::BuyXP:
		ShopManager->BuyXP(PlayerLevel, PlayerGold);
		break;
	case EPCShopRequestTypes::Reroll:
		ShopManager->Reroll(PlayerGold);
		break;
	case EPCShopRequestTypes::SellUnit:
		ShopManager->SellUnit(UnitName, UnitStarCount, PlayerGold);
		break;
	default:
		break;
	}
}
