// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "PCUnitCombatTextActor.generated.h"

class UPCUnitCombatText;
class UWidgetComponent;

USTRUCT()
struct FCombatTextInitParams
{
	GENERATED_BODY()
	
	float Value = 0.f;
	bool bCritical = false;
	FGameplayTag CombatTextTypeTag = FGameplayTag::EmptyTag;
	
	float RandomXYJitter = 12.f;
	float VerticalOffset = 40.f;
	
	FName AttachSocketName = TEXT("pelvis");
};

UCLASS()
class PROJECTPC_API APCUnitCombatTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APCUnitCombatTextActor();

	void InitCombatTextActor(const USceneComponent* InAttachComp, const FCombatTextInitParams& InParams);

	void ReturnToPool();

protected:
	UFUNCTION()
	void OnWidgetFinished();
	
	UPROPERTY(VisibleAnywhere, Category="CombatText")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY()
	TWeakObjectPtr<AActor> LiftOwner;

	UPROPERTY()
	TObjectPtr<UPCUnitCombatText> CachedWidget;
};
