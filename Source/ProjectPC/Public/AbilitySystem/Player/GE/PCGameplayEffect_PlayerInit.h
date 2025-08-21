// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PCGameplayEffect_PlayerInit.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameplayEffect_PlayerInit : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UPCGameplayEffect_PlayerInit();

private:
	float InitPlayerLevel = 1.f;
	float InitPlayerXP = 0.f;
	float InitPlayerGold = 0.f;
	float InitPlayerHP = 100.f;
};
