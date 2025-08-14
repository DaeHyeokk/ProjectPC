// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"

#include "Net/UnrealNetwork.h"


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

void UPCPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerXP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerGold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCPlayerAttributeSet, PlayerHP, COND_None, REPNOTIFY_Always);
}
