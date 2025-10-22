// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCItemSpawnSubsystem.h"

#include "Item/PCItemCapsule.h"


void UPCItemSpawnSubsystem::InitializeItemCapsuleClass(TSubclassOf<APCItemCapsule> NewItemCapsuleClass)
{
	if (NewItemCapsuleClass)
	{
		ItemCapsuleClass = NewItemCapsuleClass;
	}
}

APCItemCapsule* UPCItemSpawnSubsystem::SpawnItemCapsule(const FTransform& SpawnTransform, int32 TeamIndex)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
		return nullptr;

	if (auto ItemCapsule = GetWorld()->SpawnActor<APCItemCapsule>(
		ItemCapsuleClass, SpawnTransform))
	{
		ItemCapsule->SetOwnerPlayer(TeamIndex);
		return ItemCapsule;
	}

	return nullptr;
}
