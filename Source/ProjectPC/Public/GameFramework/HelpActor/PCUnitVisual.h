// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "PCUnitVisual.generated.h"



UCLASS()
class PROJECTPC_API APCUnitVisual : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCUnitVisual();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;

	void Init(FGameplayTag UnitID) { UnitID = InUnitID;}
	void SetUnitID(FGameplayTag UnitID) { UnitID = InUnitID; }

private:
	UPROPERTY()
	FGameplayTag InUnitID;
	
	

	
	
};
