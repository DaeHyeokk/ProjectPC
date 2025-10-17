// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace1D.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "PCCarouselHeroAnimInstance.generated.h"

class APCCarouselHeroCharacter;
class UPCDataAsset_UnitAnimSet;
class UCharacterMovementComponent;
class UBlendSpace1D;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCCarouselHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> MoveForwardSequence;
	
	void ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet);
	
public:
	void InitAnimData(const UPCDataAsset_UnitAnimSet* AnimSet);
};
