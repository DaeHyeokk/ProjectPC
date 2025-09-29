// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCUnitDamageTextSpawnSubsystem.h"


void UPCUnitDamageTextSpawnSubsystem::InitDamageTextSpawnSubsystem(
	const TSoftClassPtr<APCUnitTakenDamageTextActor>& InDamageTextActorClass)
{
	if (InDamageTextActorClass)
	{
		DamageTextActorClass = InDamageTextActorClass.LoadSynchronous();

		// 지정한 갯수만큼 미리 생성
		for (int8 i=0; i<PrewarmCount; ++i)
		{
			if (APCUnitTakenDamageTextActor* DamageTextActor = SpawnNewDamageTextActor())
				DamageTextActor->ReturnToPool();
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Nullptr DamageTextActorClass"));
}

void UPCUnitDamageTextSpawnSubsystem::SpawnDamageText(const USceneComponent* AttachToComp,
	const FDamageTextInitParams& InitParams, AActor* Owner, AActor* Instigator)
{
	if (!DamageTextActorClass)
		return;

	if (APCUnitTakenDamageTextActor* DamageText = GetDamageTextActor())
	{
		DamageText->InitDamageTextActor(AttachToComp, InitParams);
	}
}

void UPCUnitDamageTextSpawnSubsystem::ReturnToPool(APCUnitTakenDamageTextActor* DamageTextActor)
{
	if (!DamageTextActor)
		return;
	
	DamageTextPool.Enqueue(DamageTextActor);
}

APCUnitTakenDamageTextActor* UPCUnitDamageTextSpawnSubsystem::GetDamageTextActor()
{
	APCUnitTakenDamageTextActor* DamageText = nullptr;
	// 큐에서 꺼내는데 큐가 비어있을 경우 액터 새로 생성
	if (!DamageTextPool.Dequeue(DamageText))
	{
		DamageText = SpawnNewDamageTextActor();
	}

	DamageText->SetActorHiddenInGame(true);
	return DamageText;
}

APCUnitTakenDamageTextActor* UPCUnitDamageTextSpawnSubsystem::SpawnNewDamageTextActor() const
{
	UWorld* World = GetWorld();
	
	if (!DamageTextActorClass || !World)
		return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bNoFail = true;

	APCUnitTakenDamageTextActor* DamageTextActor = World->SpawnActor<APCUnitTakenDamageTextActor>(
		DamageTextActorClass,
		FTransform::Identity,
		Params);
	
	return DamageTextActor;
}
