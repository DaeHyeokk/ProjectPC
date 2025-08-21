// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerRowData.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPlayerRowData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	UImage* PlayerIcon;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLeader = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReady = false;
	
	
	
	
};
