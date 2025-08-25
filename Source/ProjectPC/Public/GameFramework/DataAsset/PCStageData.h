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
	Shop UMETA(DisplayName = "Shop"),
	PvP UMETA(DisplayName = "PvP"),
	PvE UMETA(DisplayName = "PvE"),
	Carousel UMETA(DisplayName = "Carousel"),
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
	
	UPROPERTY(EditAnywhere)
	TArray<FStageSpec> Stages;

	UPROPERTY(EditAnywhere)
	float DefaultsReadySeconds = 5.f;

	UPROPERTY(EditAnywhere)
	float DefaultsStartSeconds = 7.f;

	UPROPERTY(EditAnywhere)
	float DefaultShopSeconds = 50.f;

	UPROPERTY(EditAnywhere)
	float DefaultPvPSeconds = 50.f;

	UPROPERTY(EditAnywhere)
	float DefaultPvESeconds = 30.f;

	UPROPERTY(EditAnywhere)
	float DefaultCarouselSeconds = 30.f;


	UFUNCTION(BlueprintPure, Category = "Schedule")
	float GetDefaultDuration(EPCStageType Type) const;

	UFUNCTION(BlueprintPure, Category = "Schedule")
	float GetRoundDuration(const FRoundStep& RoundStep) const;
	
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void BuildFlattenedPhase(TArray<FRoundStep>& RoundsStep, TArray<int32>& StageIdx, TArray<int32> RoundIdx, TArray<int32>& StepIdxInRound) const;
	
	UFUNCTION(BlueprintPure, Category = "UI")
	FString MakeStageRoundLabel(int32 FloatIndex, const TArray<int32>& StageIdx, const TArray<int32>& RoundIdx) const;
	
	
};
