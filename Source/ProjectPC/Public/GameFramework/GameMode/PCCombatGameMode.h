// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PCCombatGameMode.generated.h"

/**
 * 
 */

enum class EPCStageType : uint8;
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

	virtual void PostSeamlessTravel() override;
	
	// Stage 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Schedule")
	UPCStageData* StageData = nullptr;

	// 중앙집결 링
	UPROPERTY(EditInstanceOnly, Category = "Refs")
	APCCarouselRing* CarouseRing = nullptr;

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

private:
	// 데이터 평탄화
	TArray<FRoundStep> FlatRoundSteps;
	TArray<int32> FlatStageIdx;
	TArray<int32> FlatRoundIdx;
	TArray<int32> FlatStepIdxInRound;
	int32 Cursor = -1;

	FTimerHandle RoundTimer;
	
private:
	void BuildHelperActor();
	void BuildStageData();
	void StartFromBeginning();

	void BeginCurrentStep();
	void EndCurrentStep();
	void AdvanceCursor();

	// 개별 Step 처리
	void Step_Start();
	void Step_Shop();
	void Step_PvP();
	void Step_PvE();
	void Step_Carousel();

	// 공동 유틸 함수
	void PlaceAllPlayersOnCarousel();
	void MovePlayersToBoardsAndCameraSet();
	int32 ResolveBoardIndex(const APCPlayerState* PlayerState) const;

	APCCombatGameState* GetCombatGameState() const { return GetGameState<APCCombatGameState>(); }
	float NowServer() const { return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f; }

	void BroadcastStageToClients(EPCStageType Stage, const FString& StageName, float Seconds);
	
	
	
};
