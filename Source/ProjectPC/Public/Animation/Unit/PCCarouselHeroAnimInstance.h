// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Picked")
	bool bIsPicked = false;
	
private:
	TWeakObjectPtr<APCCarouselHeroCharacter> CachedCarouselHero;
	
	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> MoveForwardSequence;

	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> IdleSequence;
	
	void ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet);
	
public:
	void InitAnimData(const UPCDataAsset_UnitAnimSet* AnimSet);
};
