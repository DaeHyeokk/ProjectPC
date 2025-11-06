// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GC/PCGameplayCue_PlayParticleAtTarget.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

static bool SpawnParticle_Internal(AActor* MyTarget, const FGameplayCueParameters& Params, UParticleSystem* InDefaultFX, FName InSocketName)
{
	if (!IsValid(MyTarget))
		return false;
	
	const UParticleSystem* ConstFX = Cast<UParticleSystem>(Params.SourceObject.Get());
	UParticleSystem* FX = const_cast<UParticleSystem*>(ConstFX);
	
	if (!FX)
		FX = InDefaultFX;
	if (!FX)
		return false;

	USceneComponent* AttachComp = Params.TargetAttachComponent.Get();
	if (!AttachComp)
	{
		FVector SpawnLoc = Params.Location;
		UGameplayStatics::SpawnEmitterAtLocation(
			MyTarget->GetWorld(),
			FX,
			SpawnLoc
			);
	}
	else
	{
		FName SocketName = InSocketName;
		if (const FHitResult* HitResult = Params.EffectContext.GetHitResult())
		{
			if (HitResult->BoneName.IsValid() && !HitResult->BoneName.IsNone())
			{
				SocketName = HitResult->BoneName;
			}
		}
		
		UGameplayStatics::SpawnEmitterAttached(
			FX,
			AttachComp,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::Type::SnapToTarget
			);
	}

	return true;
}

bool UPCGameplayCue_PlayParticleAtTarget::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	Super::OnExecute_Implementation(MyTarget, Parameters);
	return SpawnParticle_Internal(MyTarget, Parameters, DefaultFX, DefaultSocketName);
}

bool UPCGameplayCue_PlayParticleAtTarget::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	return SpawnParticle_Internal(MyTarget, Parameters, DefaultFX, DefaultSocketName);
}
