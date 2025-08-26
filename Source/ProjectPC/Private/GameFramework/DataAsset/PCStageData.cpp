// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/DataAsset/PCStageData.h"



float UPCStageData::GetDefaultDuration(EPCStageType Type) const
{
	switch (Type)
	{
	case EPCStageType::Start : return DefaultsStartSeconds;
	case EPCStageType::Carousel: return DefaultCarouselSeconds;
	case EPCStageType::Shop : return DefaultShopSeconds;
	case EPCStageType::PvP : return DefaultPvPSeconds;
	case EPCStageType::PvE : return DefaultPvESeconds;
	default: return DefaultShopSeconds;
	}
}

float UPCStageData::GetRoundDuration(const FRoundStep& RoundStep) const
{
	return (RoundStep.DurationOverride > 0.f) ? RoundStep.DurationOverride : GetDefaultDuration(RoundStep.StageType);
}

void UPCStageData::BuildFlattenedPhase(TArray<FRoundStep>& RoundsStep, TArray<int32>& StageIdx, TArray<int32> RoundIdx,
	TArray<int32>& StepIdxInRound) const
{
	RoundsStep.Reset();
	StageIdx.Reset();
	RoundIdx.Reset();
	StepIdxInRound.Reset();

	for (int32 SIdx = 0; SIdx<StageIdx.Num(); ++SIdx)
	{
		const FStageSpec& Stage = Stages[SIdx];
		for (int32 RIdx = 0; RIdx<Stage.Rounds.Num(); ++RIdx)
		{
			const FRoundSpec& Round = Stage.Rounds[RIdx];
			for (int32 k = 0; k<Round.Steps.Num(); ++k)
			{
				RoundsStep.Add(Round.Steps[k]);
				StageIdx.Add(SIdx);
				RoundIdx.Add(RIdx);
				StepIdxInRound.Add(k);
			}
		}
	}
}

FString UPCStageData::MakeStageRoundLabel(int32 FloatIndex, const TArray<int32>& StageIdx,
	const TArray<int32>& RoundIdx) const
{
	if (!StageIdx.IsValidIndex(FloatIndex) || !RoundIdx.IsValidIndex(FloatIndex))
		return TEXT("");

	const int32 StageName = StageIdx[FloatIndex];
	const int32 RoundName = RoundIdx[FloatIndex];
	return FString::Printf(TEXT("%d-%d"), StageName+1, RoundName+1);
}
