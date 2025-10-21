// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/FrameWork/PCStageData.h"
#include "GameFramework/GameModeBase.h"
#include "PCCombatGameMode.generated.h"

enum class EPCStageType : uint8;

struct FGameplayTag;
struct FRoundStep;

class APCPlayerBoard;
class APCBaseUnitCharacter;
class UPCTileManager;
class APCCombatManager;
class UPCDataAsset_UnitGEDictionary;
class APCCombatGameState;
class APCCombatBoard;
class APCCarouselRing;
class UPCStageData;
class APCPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APCCombatGameMode();
	
	// Stage 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Schedule")
	UPCStageData* StageData;

	// 중앙집결 링
	UPROPERTY(EditInstanceOnly, Category = "Refs")
	APCCarouselRing* CarouselRing = nullptr;

	// 보드 래퍼런스
	UPROPERTY(VisibleInstanceOnly, Category = "Refs")
	TArray<APCCombatBoard*> CombatBoard;

	// 회전초밥시 카메라 세팅
	UPROPERTY(EditAnywhere, Category="Camera")
	float CentralCameraBlend = 0.f;

	// 기본 보드 카메라 세팅
	UPROPERTY(EditAnywhere, Category="Camera")
	float TravelCameraBlend = 0.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float ReturnCameraBlend = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	// 좌석배정 유틸함수
	int32 GetTotalSeatSlots() const;

	// playerAttribute Delegate register
	UFUNCTION()
	void BindPlayerAttribute();

	UFUNCTION()
	void BindPlayerMainHuD();

private:
	// 데이터 평탄화
	TArray<FRoundStep> FlatRoundSteps;
	TArray<int32> FlatStageIdx;
	TArray<int32> FlatRoundIdx;
	TArray<int32> FlatStepIdxInRound;
	int32 Cursor = -1;

	FTimerHandle StartTimer;
	FTimerHandle RoundTimer;
	FTimerHandle CameraSetupTimer;
	FTimerHandle WaitAllPlayerController;
	
	// 내부 빌드 / 흐름
	void BuildHelperActor();
	void BuildStageData();
	void StartFromBeginning();
	void AdvanceCursor();
	void BeginCurrentStep();
	void EndCurrentStep();
	
	// Start 헬퍼 함수
	void PlayerStartUnitSpawn();
	
	// 개별 Step 처리
	void Step_Start();
	void Step_Setup();
	void Step_Travel();
	void Step_Return();
	void Step_PvP();
	void Step_PvE();
	void Step_CreepSpawn();
	void Step_Carousel();
	
	// 공동 유틸 함수
	void InitializeHomeBoardsForPlayers();
	void TryPlacePlayersAfterTravel();
	void PlaceAllPlayersOnCarousel();
	void PlaceAllPlayersPickUpUnit();
	void MovePlayersToBoardsAndCameraSet();
	void SetCarouselCameraForAllPlayers();
	int32 ResolveBoardIndex(const APCPlayerState* PlayerState) const;

	// 인접 스텝 조회
	const FRoundStep* PeekPrevStep() const;
	const FRoundStep* PeekNextStep() const;
	
	// CombatManager / GameState 핸들러
	UPROPERTY(VisibleInstanceOnly, Category = "Ref")
	APCCombatManager* CombatManager;
	APCCombatManager* GetCombatManager();
	APCPlayerState* FindPlayerStateBySeat(int32 SeatIdx);
	APCCombatGameState* GetCombatGameState() const;
	float NowServer() const { return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f; }

// PCPlayerBoard 관련
	// 수집한 보드 목록 / 맵
	UPROPERTY()
	TArray<APCPlayerBoard*> AllPlayerBoards;

	UPROPERTY()
	TMap<int32, APCPlayerBoard*> SeatToPlayerBoard;

	// 보드 수집 & 맵 구성
	void CollectPlayerBoards();

	// 보드 PlayerState에 세팅
	void BindPlayerBoardsToPlayerStates();

	// 좌석인덱스로 -> PlayerBoard 찾기
	APCPlayerBoard* FindPlayerBoardBySeat(int32 SeatIndex) const;
	
// ==== Unit 관련 =====
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TObjectPtr<UPCDataAsset_UnitGEDictionary> UnitGEDictionary;
	
	// 데이터 로딩

private:
	bool bSeatsFinalized = false;
	FTimerHandle ThWaitReady;
	bool IsRoundSystemReady(FString& WhyNot) const;
	void StartWhenReady();
	void AssignSeatDeterministicOnce();

	int32 ExpectedPlayers = 0;
	int32 ArrivedPlayers = 0;
	bool bTriggeredAfterTravel = false;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void OnOnePlayerArrived();

	// ── Carousel wave open schedule ──────────────────────────────
protected:
	// 웨이브 시트(두 명씩 / 마지막 웨이브 8초)
	TArray<TArray<int32>> CarouselWaves; // 각 웨이브에 열 SeatIndex 모음

	FTimerHandle ThCarouselWave;         // 5초마다(마지막 8초) 다음 웨이브
	FTimerHandle ThCarouselWaveStageUI;  // UI 카운트다운(서브 웨이브 타이머 표시용)
	int32 CurrentWaveIdx = -1;

	// 내부 헬퍼
	void BuildCarouselWavesByHP(TArray<TArray<int32>>& OutWaves);
	void StartCarouselWaves();
	void OpenCarouselWave(int32 WaveIdx);
	void FinishCarouselRound();

	// 서브 웨이브 타이머를 UI에 1초 단위로 보여주고 싶으면 이걸로 GameState StageRuntime 갱신
	void StartSubWaveTimerUI(float DurationSeconds);
};



