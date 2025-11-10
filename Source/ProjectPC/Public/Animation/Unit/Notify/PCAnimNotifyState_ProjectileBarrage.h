// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PCAnimNotifyState_ProjectileBarrage.generated.h"

UENUM(BlueprintType)
enum class EProjectileFireMode : uint8
{
	Single		UMETA(DisplayName="Single Socket"),
	AlternateLR UMETA(DisplayName="Alternate L/R"),
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCAnimNotifyState_ProjectileBarrage : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void SpawnProjectileAndFX(USkeletalMeshComponent* MeshComp);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile")
	float FireInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile")
	EProjectileFireMode FireMode = EProjectileFireMode::AlternateLR;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile", meta=(EditCondition="FireMode==EProjectileFireMode::Single",
		EditConditionHides))
	FName SingleMuzzleSocket = TEXT("Muzzle");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile", meta=(EditCondition="FireMode==EProjectileFireMode::AlternateLR",
		EditConditionHides))
	FName LeftMuzzleSocket = TEXT("Muzzle_L");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile", meta=(EditCondition="FireMode==EProjectileFireMode::AlternateLR",
	EditConditionHides))
	FName RightMuzzleSocket = TEXT("Muzzle_R");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile||TypeTags", meta=(Categories="Unit.Type"))
	FGameplayTag UnitTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile||TypeTags", meta=(Categories="Unit.Ability.Attack"))
	FGameplayTag ProjectileTypeTag;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetUnitKeyName = TEXT("TargetUnit");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FX")
	TObjectPtr<UParticleSystem> MuzzleFX = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Event")
	FGameplayTag EventTag = UnitGameplayTags::Unit_Event_SpawnProjectileSucceed;

private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FTimerHandle> TimerMap;
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, bool> UseLeftMap;
};
