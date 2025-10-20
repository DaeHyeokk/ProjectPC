// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/ExecutionCalculation/PCPlayerRoundRewardExec.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


UPCPlayerRoundRewardExec::UPCPlayerRoundRewardExec()
{
	PlayerGold.AttributeToCapture = UPCPlayerAttributeSet::GetPlayerGoldAttribute();
	PlayerGold.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	PlayerGold.bSnapshot = false;

	RelevantAttributesToCapture.Add(PlayerGold);
}

void UPCPlayerRoundRewardExec::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	int32 Stage = 1;
	int32 Round = 1;
	float Gold = 0;
	int32 PlayerWinningStreak = 0;
	int32 ResultReward = 0;

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PlayerGold, EvalParams, Gold);
	
	if (auto SourceASC = Spec.GetContext().GetOriginalInstigatorAbilitySystemComponent())
	{
		if (auto World = SourceASC->GetWorld())
		{
			if (auto GS = Cast<APCCombatGameState>(World->GetGameState()))
			{
				Stage = GS->StageRuntimeState.StageIdx;
				Round = GS->StageRuntimeState.RoundIdx;
			}
		}
		
		if (auto PS = Cast<APCPlayerState>(SourceASC->GetOwnerActor()))
		{
			PlayerWinningStreak = PS->GetPlayerWinningStreak();
		}
	}

	// 스테이지/라운드에 따른 추가 골드 획득
	if (Stage == 1)
	{
		ResultReward += Round - 1;
		if (Round == 2)
			ResultReward++;
	}
	else
	{
		if (Stage == 2 && Round == 1)
			ResultReward += 4;
		else
			ResultReward += 5;
	}

	// 골드 보유량에 따른 추가 골드 획득
	if (Gold < 60)
		ResultReward += static_cast<int32>(Gold) / 10;
	else
		ResultReward += 5;

	// 연승/연패에 따른 추가 골드 획득
	if (PlayerWinningStreak >=2 && PlayerWinningStreak <= 4)
		ResultReward += 1;
	else if (PlayerWinningStreak == 5)
		ResultReward += 2;
	else if (PlayerWinningStreak == 6)
		ResultReward += 3;

	OutExecutionOutput.AddOutputModifier(
	FGameplayModifierEvaluatedData(
		UPCPlayerAttributeSet::GetPlayerGoldAttribute(),
		EGameplayModOp::Additive,
		ResultReward
		)
	);
}
