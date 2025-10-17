#include "Utility/PCUnitCombatUtils.h"

const FIntPoint PCUnitCombatUtils::EvenRowDirections[6] = {
	{0, 1},  {1, 1},
	{-1, 0}, {1, 0},
	{0, -1}, {1, -1}
};

const FIntPoint PCUnitCombatUtils::OddRowDirections[6] = {
	{-1, 1}, {0, 1},
	{-1, 0}, {1, 0},
	{-1, -1},{0, -1}
};