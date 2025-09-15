// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Unit/PCPreviewHeroActor.h"
#include "PCPreviewHeroAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPreviewHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, Category="Snapshot")
	FPoseSnapshot SourceSnapshot;
	
private:
	TWeakObjectPtr<APCPreviewHeroActor> CachedPreviewHero;
};
