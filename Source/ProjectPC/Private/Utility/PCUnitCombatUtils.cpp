#include "Utility/PCUnitCombatUtils.h"

const TArray<FIntPoint> PCUnitCombatUtils::EvenRowDirectionArray = {
	{0, 1},  {1, 1},
	{-1, 0}, {1, 0},
	{0, -1}, {1, -1}
};

const TArray<FIntPoint> PCUnitCombatUtils::OddRowDirectionArray = {
	{-1, 1}, {0, 1},
	{-1, 0}, {1, 0},
	{-1, -1},{0, -1}
};