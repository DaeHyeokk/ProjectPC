// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCStageData.generated.h"

/**
 * 
 */

UENUM()
enum class EPCStageType : uint8
{
	Start UMETA(DisplayName = "Start Stage"),
	Setup UMETA(DisplayName = "Setup"),
	Travel UMETA(DisplayName = "Travel"),
	PvP UMETA(DisplayName = "PvP"),
	PvE UMETA(DisplayName = "PvE"),
	Carousel UMETA(DisplayName = "Carousel"),
	Return UMETA(DisplayName = "Return"),
};

USTRUCT(BlueprintType)
struct FRoundStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	EPCStageType StageType = EPCStageType::Carousel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	float DurationOverride = -1.f;
	
};

USTRUCT(BlueprintType)
struct FRoundSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	FName RoundName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	TArray<FRoundStep> Steps;
	
};

USTRUCT(BlueprintType)
struct FStageSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName StageName = "Stage";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	TArray<FRoundSpec> Rounds;
};

UCLASS()
class PROJECTPC_API UPCStageData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Schedule")
	TArray<FStageSpec> Stages;
	
	// === 기본 시간 값 (초) ===
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultStartSeconds     = 5.f;   // 입장
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultSetupSeconds     = 30.f;  // 정비
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultTravelSeconds    = 5.f;   // pvp 이동/로딩
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultPvPSeconds       = 30.f;
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultPvESeconds       = 30.f;
	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultReturnSeconds    = 3.f;

	// 캐러셀 길이 산정: 게이트 공식 or 고정값
	UPROPERTY(EditAnywhere, Category="Carousel")
	bool  bCarouselUseGateFormula = true;
	UPROPERTY(EditAnywhere, Category="Carousel")
	int32 CarouselNumPlayers      = 8;     // 인원수
	UPROPERTY(EditAnywhere, Category="Carousel")
	float CarouselGateSeconds     = 5.f;   // 1인당 5초
	UPROPERTY(EditAnywhere, Category="Carousel")
	float DefaultCarouselSeconds  = 30.f;  // bCarouselUseGateFormula=false 일 때 사용
	UPROPERTY(EditAnywhere, Category="Carousel")
	float CarouselTravelSeconds   = 3.f;  

	UFUNCTION(BlueprintPure, Category = "Schedule")
	float GetDefaultDuration(EPCStageType Type) const;

	UFUNCTION(BlueprintPure, Category = "Schedule")
	float GetRoundDuration(const FRoundStep& RoundStep) const;
	
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void BuildFlattenedPhase(TArray<FRoundStep>& RoundsStep, TArray<int32>& StageIdx, TArray<int32>& RoundIdx, TArray<int32>& StepIdxInRound) const;
	
	UFUNCTION(BlueprintPure, Category = "UI")
	FString MakeStageRoundLabel(int32 FloatIndex, const TArray<int32>& StageIdx, const TArray<int32>& RoundIdx) const;
	
	
};
