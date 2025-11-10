// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "PCGameplayCue_PlayParticleAtTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameplayCue_PlayParticleAtTarget : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="FX")
	TObjectPtr<UParticleSystem> DefaultFX = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="FX")
	FName DefaultSocketName = TEXT("Root");
	
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
