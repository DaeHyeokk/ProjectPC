// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GC/PCGameplayCue_LevelUp.h"

#include "Kismet/GameplayStatics.h"


bool UPCGameplayCue_LevelUp::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	UE_LOG(LogTemp, Warning, TEXT("Execute GC Level up!"));
	if (!LevelUpFX || !IsValid(MyTarget))
		return false;
	
	if (USceneComponent* AttachComp = Parameters.TargetAttachComponent.Get())
	{
		UGameplayStatics::SpawnEmitterAttached(
			LevelUpFX,
			AttachComp,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::Type::SnapToTarget,
			true,
			EPSCPoolMethod::AutoRelease,
			true);

		return true;
	}

	// 부착할 데가 없으면 위치 재생
	UGameplayStatics::SpawnEmitterAtLocation(
		MyTarget->GetWorld(),
		LevelUpFX,
		MyTarget->GetActorTransform(),
		/*bAutoDestroy=*/true
	);

	return true;
}