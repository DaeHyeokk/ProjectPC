// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PCItemCapsule.h"

#include "AbilitySystemComponent.h"
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
	// RewardTag를 재료 아이템 or 골드로 설정
	if (FMath::RandHelper(10) >= 3)
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
				// 골드 5원 획득 후, 아이템 캡슐 소멸
				TargetPlayer->AddValueToPlayerStat(RewardTag, 5);
				Destroy();
			}
			else
			{
				if (TargetPlayer->GetPlayerInventory()->AddItemToInventory(RewardTag))
				{
					// 아이템 획득에 성공했으면, 아이템 캡슐 소멸
					Destroy();
				}
			}

			if (auto ASC = TargetPlayer->GetAbilitySystemComponent())
			{
				ASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_Player_ItemCapsuleOpen);
			}
		}
	}
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
				AddRewardToPlayer(PS);
			}
		}
	}
}
