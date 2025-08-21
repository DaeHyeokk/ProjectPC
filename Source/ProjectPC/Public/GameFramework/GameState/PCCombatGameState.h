// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Shop/PCShopUnitData.h"
#include "Shop/PCShopUnitProbabilityData.h"
#include "PCCombatGameState.generated.h"

enum class EPCShopRequestTypes : uint8;

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APCCombatGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShopManager")
	class UPCShopManager* ShopManager;
	
	//void Server_ShopRequest(EPCShopRequestTypes RequestType);

#pragma region DataTable

protected:
	// 유닛 데이터가 저장된 DataTable 가져옴
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* ShopUnitDataTable;

	// 유닛 확률 데이터가 저장된 DataTable 가져옴
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* ShopUnitProbabilityDataTable;
	
private:
	// 실제로 DataTable에서 가져온 정보를 저장할 배열
	TArray<FPCShopUnitData> ShopUnitDataList;
	TArray<FPCShopUnitProbabilityData> ShopUnitProbabilityDataList;

	template<typename T>
	void LoadDataTable(UDataTable* DataTable, TArray<T>& OutDataList, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutDataList.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		// DataTable의 Row수만큼 메모리 미리 확보
		OutDataList.Reserve(RowPtrs.Num());
		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				OutDataList.Add(*Row);
			}
		}
	}

public:
	TArray<FPCShopUnitData>& GetShopUnitDataList();
	const TArray<FPCShopUnitProbabilityData>& GetShopUnitProbabilityDataList();
	
#pragma endregion DataTable
};
