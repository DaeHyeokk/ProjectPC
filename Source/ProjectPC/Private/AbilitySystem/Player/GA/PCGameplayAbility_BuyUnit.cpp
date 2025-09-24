// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyUnit.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_BuyUnit::UPCGameplayAbility_BuyUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyUnit);

	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_GA_Shop_BuyUnit;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UPCGameplayAbility_BuyUnit::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo->IsNetAuthority() || !CostGameplayEffectClass)
	{
		return false;
	}
	
	return true;
}

bool UPCGameplayAbility_BuyUnit::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}

void UPCGameplayAbility_BuyUnit::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpecHandle.IsValid())
	{
		CostSpecHandle.Data->SetSetByCallerMagnitude(CostTag, -UnitCost);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
	}
}

void UPCGameplayAbility_BuyUnit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	auto* PS = ActorInfo->PlayerController->GetPlayerState<APCPlayerState>();
	if (!PS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	SlotIndex = static_cast<int32>(TriggerEventData->EventMagnitude);
	UnitTag = PS->GetShopSlots()[SlotIndex].Tag;
	UnitCost = static_cast<float>(PS->GetShopSlots()[SlotIndex].UnitCost);

	const auto* CostAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCPlayerAttributeSet>();
	if (!CostAttributeSet || CostAttributeSet->GetPlayerGold() < UnitCost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) 
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (auto GS = GetWorld()->GetGameState<APCCombatGameState>())
	{
		APCCombatBoard* Board = GS->GetBattleBoardForSeat(PS->SeatIndex);

		if (!Board)
		{
			Board = GS->GetBoardBySeat(PS->SeatIndex);
		}

		if (Board)
		{
			auto BenchIndex = Board->GetFirstEmptyBenchIndex(PS->SeatIndex);
			if (BenchIndex != INDEX_NONE)
			{
				GS->GetShopManager()->BuyUnit(PS, SlotIndex, UnitTag, BenchIndex);
			}
		}
		
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_BuyUnit::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
