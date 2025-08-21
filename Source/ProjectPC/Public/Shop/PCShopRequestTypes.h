// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "CoreMinimal.h"
#include "PCShopRequestTypes.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPCShopRequestTypes : uint8
{
	None UMETA(DisplayName = "None"),
	UpdateSlot UMETA(DisplayName = "UpdateSlot"),
	BuyUnit UMETA(DisplayName = "BuyUnit"),
	BuyXP UMETA(DisplayName = "BuyXP"),
	Reroll UMETA(DisplayName = "Reroll"),
	SellUnit UMETA(DisplayName = "SellUnit"),
};
