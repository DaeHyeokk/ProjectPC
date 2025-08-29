// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCUnitAnimInstance.h"

#include "BaseGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"

void UPCUnitAnimInstance::PlayLevelStartMontage()
{
	UAnimMontage* LevelStart = CurrentAnimSet ? CurrentAnimSet->MontageByTagMap.FindRef(UnitGameplayTags::Unit_Montage_LevelStart) : nullptr;
	if (!LevelStart)
		return;

	Montage_Play(LevelStart, 1.f);
}

void UPCUnitAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	CachedUnitCharacter = Cast<APCBaseUnitCharacter>(TryGetPawnOwner());
	if (CachedUnitCharacter.IsValid())
	{
		CachedMovementComp = CachedUnitCharacter->GetCharacterMovement();
		SetAnimSet(CachedUnitCharacter->GetUnitAnimSetDataAsset());

		PlayLevelStartMontage();
	}
}

void UPCUnitAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedUnitCharacter.IsValid())
	{
		CachedUnitCharacter = Cast<APCBaseUnitCharacter>(TryGetPawnOwner());
		if (CachedUnitCharacter.IsValid())
		{
			CachedMovementComp = CachedUnitCharacter->GetCharacterMovement();
			SetAnimSet(CachedUnitCharacter->GetUnitAnimSetDataAsset());
		}
	}
	if (!CachedUnitCharacter.IsValid() || !CachedMovementComp.IsValid())
		return;

	Speed = CachedUnitCharacter->GetVelocity().Size2D();
	bIsFalling = CachedMovementComp->IsFalling();
	bIsAccelerating = CachedUnitCharacter->GetVelocity().SizeSquared2D() > 0.1f;
	Direction =  UKismetAnimationLibrary::CalculateDirection(CachedUnitCharacter->GetVelocity(), CachedUnitCharacter->GetActorRotation());
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
	Idle = AnimSet->LocomotionSet.Idle.LoadSynchronous();
	JumpStart = AnimSet->LocomotionSet.JumpStart.LoadSynchronous();
	JumpLoop = AnimSet->LocomotionSet.JumpLoop.LoadSynchronous();
	JumpLand = AnimSet->LocomotionSet.JumpLand.LoadSynchronous();
	JumpRecovery = AnimSet->LocomotionSet.JumpRecovery.LoadSynchronous();

	/*
	MontageByTagMap.Empty();
	for (const auto& KeyValuePair : AnimSet->MontageByTagMap)
	{
		if (UAnimMontage* Montage = KeyValuePair.Value)
		{
			MontageByTagMap.Add(KeyValuePair.Key, Montage);
		}
	}
	*/
}