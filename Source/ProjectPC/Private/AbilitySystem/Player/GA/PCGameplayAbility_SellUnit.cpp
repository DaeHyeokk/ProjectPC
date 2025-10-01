// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_SellUnit.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_SellUnit::UPCGameplayAbility_SellUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_SellUnit);

	ActivationRequiredTags.AddTag(PlayerGameplayTags::Player_State_Normal);
	
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Dead);
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Carousel);
	
	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_GA_Shop_SellUnit;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UPCGameplayAbility_SellUnit::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	if (!ActorInfo->IsNetAuthority())
	{
		return false;
	}
	
	return true;
}

void UPCGameplayAbility_SellUnit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                  const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	auto* Unit = const_cast<APCHeroUnitCharacter*>(Cast<APCHeroUnitCharacter>(TriggerEventData->OptionalObject));
	if (!Unit)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	auto* GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	auto UnitTag = Unit->GetUnitTag();
	auto UnitCost = GS->GetShopManager()->GetUnitCostByTag(UnitTag);
	auto UnitLevel = Unit->GetUnitLevel();
	auto SellingPrice = GS->GetShopManager()->GetSellingPrice({UnitCost, UnitLevel});
	
	if (auto PS = Cast<APCPlayerState>(ActorInfo->OwnerActor.Get()))
	{
		if (auto PC = Cast<APCCombatPlayerController>(PS->GetPlayerController()))
		{
			if (auto PlayerBoard = PC->GetPlayerBoard())
			{
				if (!PlayerBoard->RemoveFromBoard(Unit))
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
					return;
				}
			
				GS->GetShopManager()->SellUnit(UnitTag, UnitLevel);
				Unit->Destroy();
			}
		}
		
	}
			
	FGameplayEffectSpecHandle GoldSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerGoldChange, GetAbilityLevel());
	if (GoldSpecHandle.IsValid())
	{
		GoldSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerGold, SellingPrice);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*GoldSpecHandle.Data.Get());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_SellUnit::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
