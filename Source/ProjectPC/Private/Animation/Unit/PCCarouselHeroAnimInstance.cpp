// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCCarouselHeroAnimInstance.h"

#include "Animation/BlendSpace1D.h"
#include "Character/Unit/PCCarouselHeroCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

void UPCCarouselHeroAnimInstance::InitAnimData(const UPCDataAsset_UnitAnimSet* AnimSet)
{
	if (AnimSet)
		ResolveAssets(AnimSet);

	CachedCarouselHero = Cast<APCCarouselHeroCharacter>(TryGetPawnOwner());
}

void UPCCarouselHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (CachedCarouselHero.IsValid())
	{
		bIsPicked = CachedCarouselHero->IsPicked();
	}
}

void UPCCarouselHeroAnimInstance::ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet)
{
	if (UBlendSpace1D* MovementBS = AnimSet->LocomotionSet.MovementBS.LoadSynchronous())
	{
		MoveForwardSequence = MovementBS->GetBlendSample(2).Animation;
	}

	IdleSequence = AnimSet->LocomotionSet.NonCombatIdle.LoadSynchronous();
}
