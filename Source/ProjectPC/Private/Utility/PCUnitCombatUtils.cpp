#include "Utility/PCUnitCombatUtils.h"

const TArray<FIntPoint> PCUnitCombatUtils::EvenRowDirections = {
	{0, 1},  {1, 1},
	{-1, 0}, {1, 0},
	{0, -1}, {1, -1}
};

const TArray<FIntPoint> PCUnitCombatUtils::OddRowDirections = {
	{-1, 1}, {0, 1},
	{-1, 0}, {1, 0},
	{-1, -1},{0, -1}
};