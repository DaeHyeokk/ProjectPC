#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Algo/RandomShuffle.h"

namespace PCUnitCombatUtils
{
	// 6방향 (좌우 + 대각선)
	extern const FIntPoint Directions[6];
	
	inline bool IsHostile(const AActor* A, const AActor* B)
	{
		const FGenericTeamId TA = FGenericTeamId::GetTeamIdentifier(A);
		const FGenericTeamId TB = FGenericTeamId::GetTeamIdentifier(B);
		return FGenericTeamId::GetAttitude(TA, TB) == ETeamAttitude::Hostile;
	}
	
	inline TArray<FIntPoint, TInlineAllocator<6>> GetRandomDirections()
	{
		TArray<FIntPoint, TInlineAllocator<6>> Out;
		Out.Append(Directions, UE_ARRAY_COUNT(Directions));
		Algo::RandomShuffle(Out);
		return Out;
	}
}
