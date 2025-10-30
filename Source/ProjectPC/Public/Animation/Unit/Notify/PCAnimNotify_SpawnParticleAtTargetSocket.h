// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PCAnimNotify_SpawnParticleAtTargetSocket.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCAnimNotify_SpawnParticleAtTargetSocket : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	TObjectPtr<UParticleSystem> Particle = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Socket")
	FName TargetSocketName = TEXT("Impact");

	UPROPERTY(EditAnywhere, Category="Transform")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Transform")
	FRotator RotationOffset = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetUnitKeyName = TEXT("TargetUnit");
};
