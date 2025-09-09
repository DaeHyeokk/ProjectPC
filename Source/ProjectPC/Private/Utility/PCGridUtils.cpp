#include "Utility/PCGridUtils.h"

const FIntPoint PCGridUtils::Directions[6] = {
	{ -1,1}, {0, 1},
	{ -1, 0}, { 1,0},
	{ -1, -1}, { 0, -1}
};