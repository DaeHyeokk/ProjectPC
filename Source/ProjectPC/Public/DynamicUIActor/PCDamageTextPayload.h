// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "PCDamageTextPayload.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDamageTextPayload : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	float Damage = 0.f;
	UPROPERTY()
	bool bIsCritical = false;
	UPROPERTY()
	FGameplayTag DamageTypeTag;
};
