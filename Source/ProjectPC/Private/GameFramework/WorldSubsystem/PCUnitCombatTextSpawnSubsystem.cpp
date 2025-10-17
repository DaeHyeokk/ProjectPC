// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCUnitCombatTextSpawnSubsystem.h"


void UPCUnitCombatTextSpawnSubsystem::InitCombatTextSpawnSubsystem(
	const TSoftClassPtr<APCUnitCombatTextActor>& InCombatTextActorClass)
{
		CombatTextActorClass = InCombatTextActorClass.LoadSynchronous();

	if (CombatTextActorClass)
	{
		// 지정한 갯수만큼 미리 생성
		for (int8 i=0; i<PrewarmCount; ++i)
		{
			if (APCUnitCombatTextActor* DamageTextActor = SpawnNewCombatTextActor())
				DamageTextActor->ReturnToPool();
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Nullptr CombatTextActorClass"));
}

void UPCUnitCombatTextSpawnSubsystem::SpawnCombatText(const USceneComponent* AttachToComp,
	const FCombatTextInitParams& InitParams, AActor* Owner, AActor* Instigator)
{
	if (!CombatTextActorClass)
		return;

	if (APCUnitCombatTextActor* CombatText = GetCombatTextActor())
	{
		CombatText->InitCombatTextActor(AttachToComp, InitParams);
	}
}

void UPCUnitCombatTextSpawnSubsystem::ReturnToPool(APCUnitCombatTextActor* DamageTextActor)
{
	if (!DamageTextActor)
		return;
	
	CombatTextPool.Enqueue(DamageTextActor);
}

APCUnitCombatTextActor* UPCUnitCombatTextSpawnSubsystem::GetCombatTextActor()
{
	APCUnitCombatTextActor* DamageText = nullptr;
	// 큐에서 꺼내는데 큐가 비어있을 경우 액터 새로 생성
	if (!CombatTextPool.Dequeue(DamageText))
	{
		DamageText = SpawnNewCombatTextActor();
	}

	DamageText->SetActorHiddenInGame(true);
	return DamageText;
}

APCUnitCombatTextActor* UPCUnitCombatTextSpawnSubsystem::SpawnNewCombatTextActor() const
{
	UWorld* World = GetWorld();
	
	if (!CombatTextActorClass || !World)
		return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bNoFail = true;

	APCUnitCombatTextActor* DamageTextActor = World->SpawnActor<APCUnitCombatTextActor>(
		CombatTextActorClass,
		FTransform::Identity,
		Params);
	
	return DamageTextActor;
}
