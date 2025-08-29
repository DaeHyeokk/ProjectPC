// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PCPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	// UPROPERTY()
	// class UCharacterMovementComponent* OwnerMovementComponent;

	float Speed;
	// bool bIsEmoting;

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const { return Speed; }
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
    FORCEINLINE bool IsMoving() const { return Speed != 0; }
	
	// UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	// FORCEINLINE bool IsEmoting() const { return bIsEmoting; }
};
