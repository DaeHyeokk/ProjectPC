// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}
