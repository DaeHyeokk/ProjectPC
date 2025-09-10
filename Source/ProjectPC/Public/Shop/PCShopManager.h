// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
#include "PCShopManager.generated.h"

class APCPlayerState;
class APCCombatGameState;

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCShopManager : public UActorComponent
{
	GENERATED_BODY()

private:
	uint8 NumSlots = 5;

public:
	void UpdateShopSlots(APCPlayerState* TargetPlayer);
	void ReturnUnitsToShop(APCCombatGameState* GS, const TArray<FPCShopUnitData>& OldSlots, const TSet<int32>& PurchasedSlots);
	void BuyUnit(APCPlayerState* TargetPlayer, int32 SlotIndex, FGameplayTag UnitTag, int32 BenchIndex);

private:
	// 누적합을 통한 확률 구현
	template<typename T>
	const T& WeightedRandomSelect(const TArray<T>& Items, T MinValue, T MaxValue, int32& Index)
	{
		T RandomValue = FMath::RandRange(MinValue, MaxValue);
		T PrefixSum = MinValue;

		for (const auto& Item : Items)
		{
			PrefixSum += Item;
			if (RandomValue < PrefixSum)
			{
				return Item;
			}

			++Index;
		}

		return Items.Last();
	}
};

