// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PCCombatGameMode.generated.h"

/**
 * 
 */

enum class EPCStageType : uint8;
class UPCDataAsset_UnitGEDictionary;
class APCCombatGameState;
struct FRoundStep;
class APCCombatBoard;
class APCCarouselRing;
class UPCStageData;
class APCPlayerState;


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
	float CentralCameraBlend = 0.4f;

	// 기본 보드 카메라 세팅
	UPROPERTY(EditAnywhere, Category="Camera")
	float ShopFocusBlend = 0.6f;

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PostSeamlessTravel() override;

	// 좌석배정 유틸함수
	void AssignSeatInitial(bool bForceReassign);
	void AssignSeatIfNeeded(class APCPlayerState* PCPlayerState);
	int32 GetTotalSeatSlots() const;
	int32 FindNextFreeSeat(int32 TotalSeats) const;

private:
	// 데이터 평탄화
	TArray<FRoundStep> FlatRoundSteps;
	TArray<int32> FlatStageIdx;
	TArray<int32> FlatRoundIdx;
	TArray<int32> FlatStepIdxInRound;
	int32 Cursor = -1;

	FTimerHandle RoundTimer;
	FTimerHandle CameraSetupTimer;
	FTimerHandle WaitAllPlayerController;
	
private:
	void BuildHelperActor();
	void BuildStageData();
	void StartFromBeginning();

	void BeginCurrentStep();
	void EndCurrentStep();
	void AdvanceCursor();

	// 개별 Step 처리
	void Step_Start();
	void Step_Setup();
	void Step_Travel();
	void Step_Return();
	void Step_PvP();
	void Step_PvE();
	void Step_Carousel();

	// 공동 유틸 함수
	void TryPlacePlayersAfterTravel();
	void PlaceAllPlayersOnCarousel();
	void MovePlayersToBoardsAndCameraSet();
	void SetCarouselCameraForAllPlayers();
	int32 ResolveBoardIndex(const APCPlayerState* PlayerState) const;
	void BroadcastStageToClients(EPCStageType Stage, const FString& StageName, float Seconds);

	APCCombatGameState* GetCombatGameState() const { return GetGameState<APCCombatGameState>(); }
	float NowServer() const { return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TObjectPtr<UPCDataAsset_UnitGEDictionary> UnitGEDictionary;
};
