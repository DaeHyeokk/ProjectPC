// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCCarouselHeroAnimInstance.h"

#include "Animation/BlendSpace1D.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

void UPCCarouselHeroAnimInstance::InitAnimData(const UPCDataAsset_UnitAnimSet* AnimSet)
{
	if (AnimSet)
		ResolveAssets(AnimSet);
}

void UPCCarouselHeroAnimInstance::ResolveAssets(const UPCDataAsset_UnitAnimSet* AnimSet)
{
	if (UBlendSpace1D* MovementBS = AnimSet->LocomotionSet.MovementBS.LoadSynchronous())
	{
		MoveForwardSequence = MovementBS->GetBlendSample(2).Animation;
	}
}
