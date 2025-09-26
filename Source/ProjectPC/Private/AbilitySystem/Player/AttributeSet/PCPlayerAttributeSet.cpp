// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void UPCPlayerAttributeSet::OnRep_PlayerLevel(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCPlayerAttributeSet, PlayerLevel, OldValue);
}

void UPCPlayerAttributeSet::OnRep_PlayerXP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCPlayerAttributeSet, PlayerXP, OldValue);
}

void UPCPlayerAttributeSet::OnRep_PlayerGold(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCPlayerAttributeSet, PlayerGold, OldValue);
}

void UPCPlayerAttributeSet::OnRep_PlayerHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCPlayerAttributeSet, PlayerHP, OldValue);
}

void UPCPlayerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetPlayerXPAttribute())
	{
		CheckLevelUp();
	}

	if (Data.EvaluatedData.Attribute == GetPlayerHPAttribute())
	{
		CheckPlayerDie();
	}
}

void UPCPlayerAttributeSet::CheckLevelUp()
{
	auto ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	auto OwningActor = ASC->GetOwnerActor();
	if (!OwningActor || !OwningActor->HasAuthority()) return;

	auto World = OwningActor->GetWorld();
	if (!World) return;

	auto GS = World->GetGameState<APCCombatGameState>();
	if (!GS) return;

	auto PlayerCurrentLevel = static_cast<int32>(GetPlayerLevel());
	auto PlayerCurrentXP = static_cast<int32>(GetPlayerXP());
	auto RequiredXP = GS->GetMaxXP(GetPlayerLevel());

	if (PlayerCurrentXP >= RequiredXP)
	{
		SetPlayerLevel(PlayerCurrentLevel + 1);
		SetPlayerXP(PlayerCurrentXP - RequiredXP);
	}
}

void UPCPlayerAttributeSet::CheckPlayerDie()
{
	auto ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	auto OwningActor = ASC->GetOwnerActor();
	if (!OwningActor || !OwningActor->HasAuthority()) return;
	
	if (GetPlayerHP() <= 0)
	{
		if (auto PC = Cast<APCPlayerState>(OwningActor))
		{
			if (PC->GetCurrentStateTag() != PlayerGameplayTags::Player_State_Dead)
			{
				PC->ChangeState(PlayerGameplayTags::Player_State_Dead);
			}
		}
	}
}

void UPCPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerXP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerGold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerHP, COND_None, REPNOTIFY_Always);
}
