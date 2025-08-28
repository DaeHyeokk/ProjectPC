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

public:
	TArray<FPCShopUnitData> ShopSlots;

	void UpdateShopSlots();
	
	const TArray<FPCShopUnitData>& GetShopSlots();
	
	void BuyXP();
	void BuyUnit();
	void SellUnit();
	void ShopLock();

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
