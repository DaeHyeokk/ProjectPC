// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailTreeNode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/DataAsset/PCStageData.h"
#include "Shop/PCShopUnitData.h"
#include "Shop/PCShopUnitProbabilityData.h"
#include "Shop/PCShopUnitSellingPriceData.h"
#include "PCCombatGameState.generated.h"

USTRUCT(BlueprintType)
struct FSpawnSubsystemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Spawner|Registry")
	TObjectPtr<class UPCDataAsset_UnitDefinitionReg> Registry = nullptr;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCCreepUnitCharacter> DefaultCreepClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCAppearanceChangedHeroCharacter> DefaultAppearanceChangedHeroClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCAppearanceFixedHeroCharacter> DefaultAppearanceFixedHeroClass;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class UPCUnitStatusBarWidget> CreepStatusBarWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class UPCHeroStatusBarWidget> HeroStatusBarWidgetClass;
	
	// == 공통 AI Controller (전 유닛 공유) ==
	UPROPERTY(EditAnywhere, Category="Spawner|AI")
	TSubclassOf<class APCUnitAIController> DefaultAIControllerClass;
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APCCombatGameState();

protected:
	virtual void BeginPlay() override;

#pragma region GameLogic
	
	// 전체 게임 로직 관련 코드
public:

	// 표 기반 플렛 인덱스
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 FloatIndex = -1;

	// 원본 스테이지 표 좌표
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 StageIdx = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RoundIdx = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 StepIdxInRound = 0;

	// 현재 Stage 정보
	UPROPERTY(Replicated, BlueprintReadOnly)
	EPCStageType CurrentStage;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float StageDuration;

	// 서버 시간 기준 종료시각(클라 카운트다운 계산)
	UPROPERTY(Replicated, BlueprintReadOnly)
	float StageEndTime_Server = 0.f;

	// 1-2, 1-3 같은 라벨로 변환 (UI 용)
	UFUNCTION(BlueprintPure, Category = "UI")
	FString GetStageRoundLabel() const;

	// 남은 시간(초) - UI 용
	UFUNCTION(BlueprintPure, Category = "UI")
	float GetRemainingSeconds() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion GameLogic
	
#pragma region Shop
	
private:
	UPROPERTY()
	class UPCShopManager* ShopManager;

public:
	FORCEINLINE UPCShopManager* GetShopManager() const { return ShopManager; }

protected:
	// 유닛 데이터가 저장된 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* ShopUnitDataTable;

	// 유닛 확률 데이터가 저장된 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* ShopUnitProbabilityDataTable;

	// 유닛 판매 가격 데이터가 저장된 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* ShopUnitSellingPriceDataTable;
	
private:
	// 실제로 DataTable에서 가져온 정보를 저장할 배열
	TArray<FPCShopUnitData> ShopUnitDataList;
	TArray<FPCShopUnitProbabilityData> ShopUnitProbabilityDataList;
	TMap<TPair<uint8, uint8>, uint8> ShopUnitSellingPriceDataMap;

	TArray<FPCShopUnitData> ShopUnitDataList_Cost1;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost2;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost3;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost4;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost5;

	// DataTable을 읽어 아웃파라미터로 TArray에 값을 넘기는 템플릿 함수
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

	// DataTable을 읽어 아웃파라미터로 TMap에 값을 넘기는 템플릿 함수
	template<typename T>
	void LoadDataTableToMap(UDataTable* DataTable, TMap<TPair<uint8, uint8>, uint8>& OutMap, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutMap.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				// Key는 (UnitCost, UnitLevel), 값은 UnitSellingPrice
				TPair<uint8, uint8> Key(Row->UnitCost, Row->UnitLevel);
				OutMap.Add(Key, Row->UnitSellingPrice);
			}
		}
	}
	
public:
	const TArray<FPCShopUnitData>& GetShopUnitDataList();
	const TArray<FPCShopUnitProbabilityData>& GetShopUnitProbabilityDataList();
	const TMap<TPair<uint8, uint8>, uint8>& GetShopUnitSellingPriceDataMap();
	TArray<float> GetCostProbabilities();
	TArray<FPCShopUnitData>& GetShopUnitDataListByCost(uint8 Cost);
	
#pragma endregion Shop

#pragma region Unit
	
private:
	UPROPERTY(EditAnywhere, Category="Unit Spawner Data")
	FSpawnSubsystemConfig SpawnConfig;

public:
	FORCEINLINE const FSpawnSubsystemConfig& GetSpawnConfig() const { return SpawnConfig; }
	
#pragma endregion Unit
};
