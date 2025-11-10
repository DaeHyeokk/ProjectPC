// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCPreloadHeroActor.h"

#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Net/UnrealNetwork.h"

APCPreloadHeroActor::APCPreloadHeroActor()
{
	bReplicates = true;
	SetReplicates(true);

	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetIsReplicated(true);
	MeshComp->SetComponentTickEnabled(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetEnableGravity(false);
}

void APCPreloadHeroActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCPreloadHeroActor, UnitTag);
	DOREPLIFETIME(APCPreloadHeroActor, UnitLevel);
}

void APCPreloadHeroActor::OnRep_UnitTag() const
{
	ApplyVisualsForUnitTag();
}

void APCPreloadHeroActor::ApplyVisualsForUnitTag() const
{
	if (!UnitTag.IsValid() || !MeshComp)
		return;

	if (UWorld* World = GetWorld())
	{
		if (auto* SpawnSubSystem = World->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			const auto* Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			if (!Definition)
			{
				SpawnSubSystem->EnsureConfigFromGameState();
				Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			}

			if (Definition)
			{
				SpawnSubSystem->ApplyDefinitionData(this, UnitLevel, Definition);
			}
		}
	}
}
