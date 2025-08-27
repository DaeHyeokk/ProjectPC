// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCUnitAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"


void UPCUnitAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (CachedCharacter.IsValid())
	{
		CachedMovementComp = CachedCharacter->GetCharacterMovement();
	}

	if (DefaultAnimSet)
	{
		SetAnimSet(DefaultAnimSet);
	}
}

void UPCUnitAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter.IsValid())
	{
		CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (CachedCharacter.IsValid())
			CachedMovementComp = CachedCharacter->GetCharacterMovement();
	}
	if (!CachedCharacter.IsValid() || !CachedMovementComp.IsValid())
		return;

	Speed = CachedCharacter->GetVelocity().Size2D();
	bIsFalling = CachedMovementComp->IsFalling();
	bIsAccelerating = CachedCharacter->GetVelocity().SizeSquared2D() > 0.1f;
	Direction =  UKismetAnimationLibrary::CalculateDirection(CachedCharacter->GetVelocity(), CachedCharacter->GetActorRotation());
	bFullBody = GetCurveValue(TEXT("FullBody")) > 0.f;
}

void UPCUnitAnimInstance::SetAnimSet(UPCDataAsset_UnitAnimSet* NewSet)
{
	if (CurrentAnimSet == NewSet || !NewSet)
		return;

	CurrentAnimSet = NewSet;
	ResolveAssets(CurrentAnimSet);
}

void UPCUnitAnimInstance::ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet)
{
	MovementBS = AnimSet->LocomotionSet.MovementBS.LoadSynchronous();
	JumpStart = AnimSet->LocomotionSet.JumpStart.LoadSynchronous();
	JumpLoop = AnimSet->LocomotionSet.JumpLoop.LoadSynchronous();
	JumpLand = AnimSet->LocomotionSet.JumpLand.LoadSynchronous();
	JumpRecovery = AnimSet->LocomotionSet.JumpRecovery.LoadSynchronous();
	
	MontageByTagMap.Empty();
	for (const auto& KeyValuePair : AnimSet->MontageByTagMap)
	{
		if (UAnimMontage* Montage = KeyValuePair.Value.LoadSynchronous())
		{
			MontageByTagMap.Add(KeyValuePair.Key, Montage);
		}
	}
}

UAnimMontage* UPCUnitAnimInstance::GetMontageByTag(FGameplayTag MontageTag)
{
	if (const TObjectPtr<UAnimMontage>* Found = MontageByTagMap.Find(MontageTag))
	{
		return Found->Get();
	}

	return nullptr;
}