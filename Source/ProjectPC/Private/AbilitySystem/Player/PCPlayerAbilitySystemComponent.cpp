// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"

#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"

#include "DataAsset/Player/PCDataAsset_PlayerAbilities.h"


void UPCPlayerAbilitySystemComponent::ApplyInitializedEffects()
{
	// GetOwner()가 유효하지 않거나, 서버가 아니라면 return
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!PlayerAbilityData || !PlayerAbilityData->InitializedEffect) return;

	// GE SpecHandle 생성
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(PlayerAbilityData->InitializedEffect, 1, MakeEffectContext());
	if (!EffectSpecHandle.IsValid()) return;
	
	// 자기 자신에게 GE 적용
	for (const auto& InitialValue : PlayerAbilityData->InitializedEffectCallerValues)
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InitialValue.Key, InitialValue.Value);
	}
	
	ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void UPCPlayerAbilitySystemComponent::ApplyInitializedAbilities()
{
	// GetOwner()가 유효하지 않거나, 서버가 아니라면 return
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const auto& InitialGAClass : PlayerAbilityData->InitializedAbilities)
	{
		auto AbilitySpec = FGameplayAbilitySpec(InitialGAClass, 0, INDEX_NONE, nullptr);
		GiveAbility(AbilitySpec);
	}
}
