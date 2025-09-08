#pragma once

#include "CoreMinimal.h"
#include "PCTileType.generated.h"

UENUM()
enum class ETileAction : uint8
{
	Reserve UMETA(DisplayName="Reserve"),
	Occupy UMETA(DisplayName="Occupy"),
	Release UMETA(DisplayName="Release"),
};

class PCTileType
{
public:
	
};
