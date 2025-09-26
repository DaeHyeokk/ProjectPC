// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/FrameWork/PCStageData.h"



float UPCStageData::GetDefaultDuration(EPCStageType Type) const
{
	switch (Type)
	{
	case EPCStageType::Start:    return DefaultStartSeconds;
	case EPCStageType::Setup:    return DefaultSetupSeconds;
	case EPCStageType::Travel:   return DefaultTravelSeconds;
	case EPCStageType::PvP:      return DefaultPvPSeconds;
	case EPCStageType::PvE:      return DefaultPvESeconds;
	case EPCStageType::CreepSpawn: return DefaultsCreepSpawnSeconds;
	case EPCStageType::Return:   return DefaultReturnSeconds;
	case EPCStageType::Carousel:
		return bCarouselUseGateFormula
			? (FMath::Max(0, CarouselNumPlayers) * FMath::Max(0.f, CarouselGateSeconds))
			: DefaultCarouselSeconds;
	default:
		return DefaultSetupSeconds;
	}
}

float UPCStageData::GetRoundDuration(const FRoundStep& RoundStep) const
{
	return (RoundStep.DurationOverride > 0.f) ? RoundStep.DurationOverride : GetDefaultDuration(RoundStep.StageType);
}

// Flattener
static void AppendRound(TArray<FRoundStep>& Steps, TArray<int32>& StageIdx, TArray<int32>& RoundIdx, TArray<int32>& StepIdxInRound,
	int32 S, int32 R, const TArray<FRoundStep>& Def)
{
	for (int32 k = 0; k < Def.Num(); ++k)
	{
		Steps.Add(Def[k]);
		StageIdx.Add(S);
		RoundIdx.Add(R);
		StepIdxInRound.Add(k);
	}
}

void UPCStageData::BuildFlattenedPhase(TArray<FRoundStep>& RoundsStep, TArray<int32>& StageIdx, TArray<int32>& RoundIdx,
	TArray<int32>& StepIdxInRound) const
{
	RoundsStep.Reset();
	StageIdx.Reset();
	RoundIdx.Reset();
	StepIdxInRound.Reset();

	// 내가 제작한 데이터에셋기준 스테이지 생성
	if (Stages.Num() > 0)
	{
		for (int32 SIdx=0; SIdx<Stages.Num(); ++SIdx)
		{
			const FStageSpec& Stage = Stages[SIdx];
			for (int32 RIdx=0; RIdx<Stage.Rounds.Num(); ++RIdx)
			{
				const FRoundSpec& Round = Stage.Rounds[RIdx];

				// 사용자가 넣은 Steps 그대로 복사, 시간은 자동 보정
				TArray<FRoundStep> FixedSteps;
				FixedSteps.Reserve(Round.Steps.Num());
				for (const FRoundStep& Src : Round.Steps)
				{
					FRoundStep S = Src;
					if (S.DurationOverride <= 0.f)
						S.DurationOverride = GetDefaultDuration(S.StageType);
					FixedSteps.Add(S);
				}

				AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, RIdx, FixedSteps);
			}
		}
		return;
	}

	// === 2) 프리셋 자동 생성: 캐러셀 앞에 무조건 Travel 포함 ===
    auto S = [&](EPCStageType T, float Sec)->FRoundStep {
        FRoundStep X; X.StageType = T;
        X.DurationOverride = (Sec > 0.f) ? Sec : GetDefaultDuration(T);
        return X;
    };

    // (0) 입장 5초
    AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, 1, 1, {
        S(EPCStageType::Start, 5.f),
    	S(EPCStageType::Return, 5.f)
    });

    // (1) Stage 1 — 1-2, 1-3, 1-4 (PvE)
    {
        const int32 SIdx = 1;

        // 1-2 : Setup 3, Travel 3, PvE 30
        AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, 2, {
            S(EPCStageType::Setup,  3.f),
            S(EPCStageType::CreepSpawn, 3.f),
            S(EPCStageType::PvE,   20.f),
        	S(EPCStageType::Return, 2.f)
        });

        // 1-3 : Setup 15, Travel 3, PvE 30
        AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, 3, {
            S(EPCStageType::Setup,  15.f),
            S(EPCStageType::CreepSpawn, 3.f),
            S(EPCStageType::PvE,    30.f),
        	S(EPCStageType::Return, 2.f)
        });

        // 1-4 : Setup 20, Travel 3, PvE 30
        AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, 4, {
            S(EPCStageType::Setup,  20.f),
            S(EPCStageType::CreepSpawn, 3.f),
            S(EPCStageType::PvE,    30.f),
        	S(EPCStageType::Return, 2.f)
        });
    }

    // (2) Stage 2 ~ 8 공통 패턴
    auto AddStage = [&](int32 SIdx)
    {
        auto AddPvp = [&](int32 R)
        {
            // 전투전 준비 30, 이동 5, 전투 30, 복귀 3
            AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, R, {
                S(EPCStageType::Setup,   30.f),
                S(EPCStageType::Travel,   DefaultTravelSeconds),
                S(EPCStageType::PvP,     30.f),
                S(EPCStageType::Return,   3.f)
            });
        };

        auto AddCarousel = [&](int32 R)
        {
            // ★ 요청: 캐러셀 전엔 무조건 Travel 포함
            const float CaroLen = bCarouselUseGateFormula
                ? (FMath::Max(0, CarouselNumPlayers) * FMath::Max(0.f, CarouselGateSeconds))
                : DefaultCarouselSeconds;

            AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, R, {
                S(EPCStageType::Travel,   CarouselTravelSeconds),
                S(EPCStageType::Carousel, CaroLen),
            	S(EPCStageType::Return, 3.f)
            });
        };

        auto AddCreep7 = [&](int32 R)
        {
            // 크립: 준비20, 카운트다운 3, 전투 30
            AppendRound(RoundsStep, StageIdx, RoundIdx, StepIdxInRound, SIdx, R, {
                S(EPCStageType::Setup,  20.f),
                S(EPCStageType::Travel,  3.f),
                S(EPCStageType::PvE,    30.f)
            });
        };

        // 1,2,3
        AddPvp(1);
        AddPvp(2);
        AddPvp(3);

        // 4 : 캐러셀(항상 Travel 포함)
        AddCarousel(4);

        // 5,6
        AddPvp(5);
        AddPvp(6);

        // 7 : 크립
        AddCreep7(7);
    };

    for (int32 SIdx=2; SIdx<=8; ++SIdx)
        AddStage(SIdx);
}

FString UPCStageData::MakeStageRoundLabel(int32 FloatIndex, const TArray<int32>& StageIdx,
	const TArray<int32>& RoundIdx) const
{
	if (!StageIdx.IsValidIndex(FloatIndex) || !RoundIdx.IsValidIndex(FloatIndex))
		return TEXT("");

	const int32 S = StageIdx[FloatIndex];
	const int32 R = RoundIdx[FloatIndex];
	return FString::Printf(TEXT("%d-%d"), S+1, R+1);
}
