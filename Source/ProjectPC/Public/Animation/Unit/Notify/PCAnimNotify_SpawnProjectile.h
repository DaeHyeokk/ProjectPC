// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PCAnimNotify_SpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCAnimNotify_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, Category="Socket")
	FName SocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, Category="Transform")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Transform")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category="Tags")
	FGameplayTag UnitTypeTag;

	UPROPERTY(EditAnywhere, Category="Tags")
	FGameplayTag ProjectileTypeTag;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetUnitKeyName = TEXT("TargetUnit");
};
