// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PCCombatGameMode.generated.h"

/**
 * 
 */

class UPCDataAsset_UnitGEDictionary;
class APCPlayerState;

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
	None,
	Carousel,
	ToSeats,
	PlayerCombating,
	CreepCombating
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPhaseChanged, ECombatPhase, NewPhase);

UCLASS()
class PROJECTPC_API APCCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

	
public:
	virtual void PostSeamlessTravel() override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TObjectPtr<UPCDataAsset_UnitGEDictionary> UnitGEDictionary;
};
