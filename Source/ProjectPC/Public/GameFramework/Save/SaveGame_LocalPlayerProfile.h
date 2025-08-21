// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGame_LocalPlayerProfile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API USaveGame_LocalPlayerProfile : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString LocalUserId;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString DisplayName;

	// 생성시간 (디버그용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FDateTime CreatedAtUTC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString LastServerAddr;
	
	
	
	
};
