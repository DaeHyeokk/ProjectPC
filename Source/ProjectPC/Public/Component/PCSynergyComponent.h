// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PCSynergyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCSynergyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPCSynergyComponent();

	void RegisterSynergy();
	
protected:
	TMap<FGameplayTag, int32> SynergyCountMap;
	
	
};
