// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PCItemCapsule.h"

#include "BaseGameplayTags.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "Item/PCPlayerInventory.h"


APCItemCapsule::APCItemCapsule()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
}

void APCItemCapsule::SetOwnerPlayer(int32 NewTeamIndex)
{
	if (!HasAuthority()) return;
	TeamIndex = NewTeamIndex;
}

void APCItemCapsule::SetRandomRewardTag()
{
	if (FMath::RandHelper(1) == 0)
	{
		if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			RewardTag = ItemManager->GetRandomBaseItem();
		}
	}
	else
	{
		RewardTag = PlayerGameplayTags::Player_Stat_PlayerGold;
	}
}

void APCItemCapsule::AddRewardToPlayer(APCPlayerState* TargetPlayer)
{
	if (!TargetPlayer) return;
	
	auto Inventory = TargetPlayer->GetPlayerInventory();
	if (Inventory->GetInventorySize() < Inventory->MaxInventorySlots)
	{
		SetRandomRewardTag();
		if (RewardTag.IsValid())
		{
			if (RewardTag == PlayerGameplayTags::Player_Stat_PlayerGold)
			{
				TargetPlayer->AddValueToPlayerStat(RewardTag, 5);
				Destroy();
				return;
			}
			else
			{
				if (TargetPlayer->GetPlayerInventory()->AddItemToInventory(RewardTag))
				{
					Destroy();
					return;
				}
			}
		}
	}
	
	SetActorEnableCollision(true);
}

void APCItemCapsule::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!HasAuthority() || this->IsActorBeingDestroyed()) return;
	
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (!OtherActor) return;
	
	if (const auto PlayerCharacter = Cast<APCPlayerCharacter>(OtherActor))
	{
		if (auto PS = PlayerCharacter->GetPlayerState<APCPlayerState>())
		{
			if (TeamIndex == PS->SeatIndex)
			{
				SetActorEnableCollision(false);
				AddRewardToPlayer(PS);
			}
		}
	}
}
