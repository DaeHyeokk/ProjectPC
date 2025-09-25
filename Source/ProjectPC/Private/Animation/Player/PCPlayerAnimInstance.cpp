// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Player/PCPlayerAnimInstance.h"

#include "GameFramework/Character.h"

#include "Character/Player/PCPlayerCharacter.h"


void UPCPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UPCPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		Speed = OwnerCharacter->GetVelocity().Length();

		if (const auto PlayerCharacter = Cast<APCPlayerCharacter>(OwnerCharacter))
		{
			bIsDead = PlayerCharacter->IsDead();
		}
	}
}

void UPCPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}