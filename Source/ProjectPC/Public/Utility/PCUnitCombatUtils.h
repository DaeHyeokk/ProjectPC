#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Algo/RandomShuffle.h"

namespace PCUnitCombatUtils
{
	// 짝수행 6방향 (좌우 + 대각선)
	extern const FIntPoint EvenRowDirections[6];
	// 홀수행 6방향
	extern const FIntPoint OddRowDirections[6];
	
	inline TArray<FIntPoint, TInlineAllocator<6>> GetRandomDirections(bool bEvenRow)
	{
		TArray<FIntPoint, TInlineAllocator<6>> Out;
		Out.Append(bEvenRow ? EvenRowDirections : OddRowDirections, UE_ARRAY_COUNT(EvenRowDirections));
		Algo::RandomShuffle(Out);
		return Out;
	}

	inline bool IsHostile(const AActor* A, const AActor* B)
	{
		const FGenericTeamId TA = FGenericTeamId::GetTeamIdentifier(A);
		const FGenericTeamId TB = FGenericTeamId::GetTeamIdentifier(B);
		return FGenericTeamId::GetAttitude(TA, TB) == ETeamAttitude::Hostile;
	}
}
