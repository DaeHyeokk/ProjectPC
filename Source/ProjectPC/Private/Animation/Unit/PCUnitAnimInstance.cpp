// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCUnitAnimInstance.h"

#include "BaseGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/GameState/PCCombatGameState.h"

void UPCUnitAnimInstance::PlayLevelStartMontage()
{
	UAnimMontage* LevelStart = CurrentAnimSet ? CurrentAnimSet->GetMontageByTag(UnitGameplayTags::Unit_Montage_LevelStart) : nullptr;
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

	if (GetWorld())
	{
		CachedCombatGameState = GetWorld()->GetGameState<APCCombatGameState>();
	}
}

void UPCUnitAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!CachedCombatGameState.IsValid() && GetWorld())
	{
		CachedCombatGameState = GetWorld()->GetGameState<APCCombatGameState>();
	}
	
	if (!CachedUnitCharacter.IsValid())
	{
		CachedUnitCharacter = Cast<APCBaseUnitCharacter>(TryGetPawnOwner());
		if (CachedUnitCharacter.IsValid())
		{
			CachedMovementComp = CachedUnitCharacter->GetCharacterMovement();
			SetAnimSet(CachedUnitCharacter->GetUnitAnimSetDataAsset());
		}
	}
	if (!CachedCombatGameState.IsValid() || !CachedUnitCharacter.IsValid() || !CachedMovementComp.IsValid())
		return;

	Speed = CachedUnitCharacter->GetVelocity().Size2D();
	bIsFalling = CachedMovementComp->IsFalling();
	bIsAccelerating = CachedUnitCharacter->GetVelocity().SizeSquared2D() > 0.1f;
	bIsCombatActive = CachedUnitCharacter->IsOnField() && CachedCombatGameState->IsCombatActive();
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
	NonCombatIdle = AnimSet->LocomotionSet.NonCombatIdle.LoadSynchronous();
	CombatIdle = AnimSet->LocomotionSet.CombatIdle.LoadSynchronous();
	JumpStart = AnimSet->LocomotionSet.JumpStart.LoadSynchronous();
	JumpLoop = AnimSet->LocomotionSet.JumpLoop.LoadSynchronous();
	JumpLand = AnimSet->LocomotionSet.JumpLand.LoadSynchronous();
	JumpRecovery = AnimSet->LocomotionSet.JumpRecovery.LoadSynchronous();
}