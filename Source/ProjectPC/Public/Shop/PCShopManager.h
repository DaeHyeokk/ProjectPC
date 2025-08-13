// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
#include "Shop/PCShopUnitProbabilityData.h"
#include "PCShopManager.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCShopManager : public UActorComponent
{
	GENERATED_BODY()

#pragma region DataTable
	
protected:
	// 유닛 데이터가 저장된 DataTable 가져옴
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	UDataTable* ShopUnitDataTable;

	// 유닛 확률 데이터가 저장된 DataTable 가져옴
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	UDataTable* ShopUnitProbabilityDataTable;

public:
	// Row네임(UnitName)을 통해 해당 UnitData를 찾아서 아웃파라미터로 넘기고, bool값 리턴
	UFUNCTION(BlueprintCallable, Category = "DataTable")
	bool GetUnitDataByRowName(FName RowName, FPCShopUnitData& OutUnitData);

	// Row네임(Level)을 통해 해당 UnitProbabilityData를 찾아서 아웃파라미터로 넘기고, bool값 리턴
	UFUNCTION(BlueprintCallable, Category = "DataTable")
	bool GetUnitProbabilityDataByRowName(FName RowName, FPCShopUnitProbabilityData& OutUnitData);

#pragma endregion DataTable

private:
	TArray<FPCShopUnitData> ShopSlots;
	
public:
	void UpdateShopSlots(uint8 PlayerLevel);
	TArray<FPCShopUnitData> GetShopSlots();
	
	bool CanBuyUnit(int32 PlayerGold);
	bool CanBuyXP(int32 PlayerGold);
	bool CanReroll(int32 PlayerGold);
	
	void BuyUnit(uint8 SlotIndex, int32& PlayerGold);
	void BuyXP(uint8& PlayerLevel, int32& PlayerGold);
	void Reroll(int32& PlayerGold);
	void SellUnit(FName UnitName, uint8 UnitStarCount, int32& PlayerGold);
};
