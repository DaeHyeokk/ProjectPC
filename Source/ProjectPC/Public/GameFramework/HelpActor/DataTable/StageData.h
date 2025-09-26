#pragma once

#include "Engine/DataTable.h"
#include "StageData.generated.h"


USTRUCT(BlueprintType)
struct FStageDamage : public  FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseDamage = 0;
	
};
