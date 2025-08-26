// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/DataAsset/PCStageData.h"
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
