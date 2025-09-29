// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShaderParameterParser.h"
#include "GameFramework/Actor.h"
#include "PCUnitTakenDamageTextActor.generated.h"

class UPCUnitTakenDamageText;
class UWidgetComponent;

USTRUCT()
struct FDamageTextInitParams
{
	GENERATED_BODY()
	
	float Damage = 0.f;
	bool bCritical = false;
	FGameplayTag DamageTypeTag = FGameplayTag::EmptyTag;
	
	float RandomXYJitter = 12.f;
	float VerticalOffset = 40.f;
	
	FName AttachSocketName = TEXT("pelvis");
};

UCLASS()
class PROJECTPC_API APCUnitTakenDamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APCUnitTakenDamageTextActor();

	void InitDamageTextActor(const USceneComponent* InAttachComp, const FDamageTextInitParams& InParams);

	void ReturnToPool();

protected:
	UFUNCTION()
	void OnWidgetFinished();
	
	UPROPERTY(VisibleAnywhere, Category="DamageText")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY()
	TWeakObjectPtr<AActor> LiftOwner;

	UPROPERTY()
	TObjectPtr<UPCUnitTakenDamageText> CachedWidget;
};
