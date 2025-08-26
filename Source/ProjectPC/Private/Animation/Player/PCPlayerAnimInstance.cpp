// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Player/PCPlayerAnimInstance.h"

#include "GameFramework/Character.h"


void UPCPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
}

void UPCPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		Speed = OwnerCharacter->GetVelocity().Length();
	}
}

void UPCPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}
