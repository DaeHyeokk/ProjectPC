// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "PCAnimNotify_SpawnProjectile.generated.h"

/**
 * 
 */

USTRUCT()
struct FTargetData_Projectile : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<APCBaseProjectile> Projectile;

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
};

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

	UPROPERTY(EditAnywhere, Category="Tags", meta=(Categories="Unit.Type"))
	FGameplayTag UnitTypeTag;

	UPROPERTY(EditAnywhere, Category="Tags", meta=(Categories="Unit.Ability.Attack"))
	FGameplayTag ProjectileTypeTag;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetUnitKeyName = TEXT("TargetUnit");
};
