// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitManaRegenGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/GameState/PCCombatGameState.h"


UPCUnitManaRegenGameplayAbility::UPCUnitManaRegenGameplayAbility()
{
	PeriodSeconds = 1.f;
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_ManaRegen);
}

void UPCUnitManaRegenGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	if (GetWorld())
	{
		CachedCombatGameState = Cast<APCCombatGameState>(GetWorld()->GetGameState());
	}
	
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UPCUnitManaRegenGameplayAbility::OnPulseTick()
{
	if (!Unit->IsOnField())
		return;
	
	if (!CachedCombatGameState.IsValid())
	{
		if (UWorld* World = GetWorld())
			if (APCCombatGameState* CombatGS = World->GetGameState<APCCombatGameState>())
				CachedCombatGameState = CombatGS;
	}

	if (!CachedCombatGameState.IsValid() || !CachedCombatGameState->GetGameStateTag().MatchesTag(GameStateTags::Game_State_Combat_Active))
		return;
	
	Super::OnPulseTick();
}
