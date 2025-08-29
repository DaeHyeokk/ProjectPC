// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace1D.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "PCUnitAnimInstance.generated.h"

class APCBaseUnitCharacter;
class UCharacterMovementComponent;
class UPCDataAsset_UnitAnimSet;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// 기본 세트 (없으면 캐릭터에서 SetAnimSet으로 주입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AnimSet")
	TObjectPtr<UPCDataAsset_UnitAnimSet> DefaultAnimSet;

	UFUNCTION(BlueprintCallable, Category="AnimSet")
	void SetAnimSet(UPCDataAsset_UnitAnimSet* NewSet);
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsFalling = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsAccelerating = false;
	
	UPROPERTY(BlueprintReadOnly, Category="AnimSlot")
	bool bFullBody = false;

	UFUNCTION(BlueprintCallable)
	void PlayLevelStartMontage();
	
protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	TWeakObjectPtr<APCBaseUnitCharacter> CachedUnitCharacter;
	TWeakObjectPtr<UCharacterMovementComponent> CachedMovementComp;

	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPCDataAsset_UnitAnimSet> CurrentAnimSet;

	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBlendSpace1D> MovementBS;

	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> Idle;
	
	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> JumpStart;
	
	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> JumpLoop;
	
	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> JumpLand;

	UPROPERTY(BlueprintReadOnly, Category="AnimSet", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequence> JumpRecovery;

	void ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet);

	bool bPlayStartAnim = false;
};
