// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Net/UnrealNetwork.h"
#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}

void APCCombatGameState::BeginPlay()
{
	Super::BeginPlay();

	if (ShopUnitDataTable && ShopUnitProbabilityDataTable)
	{
		LoadDataTable<FPCShopUnitData>(ShopUnitDataTable, ShopUnitDataList, TEXT("Loading Shop Unit Data"));
		LoadDataTable<FPCShopUnitProbabilityData>(ShopUnitProbabilityDataTable, ShopUnitProbabilityDataList, TEXT("Loading Shop Unit Probability Data"));
	}
}

FString APCCombatGameState::GetStageRoundLabel() const
{
	return FString::Printf(TEXT("%d-%d"), StageIdx+1, RoundIdx+1);
}

float APCCombatGameState::GetRemainingSeconds() const
{
	return FMath::Max(0.f, StageEndTime_Server - GetServerWorldTimeSeconds());
}

void APCCombatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCCombatGameState, FloatIndex);
	DOREPLIFETIME(APCCombatGameState, StageIdx);
	DOREPLIFETIME(APCCombatGameState, RoundIdx);
	DOREPLIFETIME(APCCombatGameState, StepIdxInRound);
	DOREPLIFETIME(APCCombatGameState, CurrentStage);
	DOREPLIFETIME(APCCombatGameState, StageDuration);
	DOREPLIFETIME(APCCombatGameState, StageEndTime_Server);
}

TArray<FPCShopUnitData>& APCCombatGameState::GetShopUnitDataList()
{
	return ShopUnitDataList;
}

const TArray<FPCShopUnitProbabilityData>& APCCombatGameState::GetShopUnitProbabilityDataList()
{
	return ShopUnitProbabilityDataList;
}
