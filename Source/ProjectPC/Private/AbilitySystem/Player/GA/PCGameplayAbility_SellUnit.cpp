// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_SellUnit.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_SellUnit::UPCGameplayAbility_SellUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_SellUnit);
	
	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_GA_Shop_SellUnit;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UPCGameplayAbility_SellUnit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo->IsNetAuthority() || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	auto* Unit = const_cast<APCHeroUnitCharacter*>(Cast<APCHeroUnitCharacter>(TriggerEventData->OptionalObject));
	if (!Unit)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	auto* GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	auto UnitTag = Unit->GetUnitTag();
	auto UnitCost = GS->GetUnitCostByTag(UnitTag);
	auto UnitLevel = Unit->GetUnitLevel();
	auto SellingPrice = GS->GetSellingPrice({UnitCost, UnitLevel});

	if (auto PS = ActorInfo->PlayerController->GetPlayerState<APCPlayerState>())
	{
		if (auto TileManager = GS->GetBoardBySeat(PS->SeatIndex)->TileManager)
		{
			GS->GetShopManager()->SellUnit(UnitTag, UnitLevel);
			Unit->Destroy();
		}
	}
			
	FGameplayEffectSpecHandle XPSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerGoldChange, GetAbilityLevel());
	if (XPSpecHandle.IsValid())
	{
		XPSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerGold, SellingPrice);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*XPSpecHandle.Data.Get());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_SellUnit::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
