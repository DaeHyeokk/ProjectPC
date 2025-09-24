// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"

#include "GameplayEffect.h"

#include "DataAsset/Player/PCDataAsset_PlayerAbilities.h"


void UPCPlayerAbilitySystemComponent::ApplyInitializedEffects()
{
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
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const auto& InitialGAClass : PlayerAbilityData->InitializedAbilities)
	{
		auto AbilitySpec = FGameplayAbilitySpec(InitialGAClass, 0, INDEX_NONE, nullptr);
		GiveAbility(AbilitySpec);
	}
}

void UPCPlayerAbilitySystemComponent::ApplyPlayerEffects(FGameplayTag GE_Tag, float GE_Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!PlayerAbilityData) return;

	if (auto PlayerGEClass = PlayerAbilityData->PlayerEffects.Find(GE_Tag))
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(*PlayerGEClass, 1, MakeEffectContext());
		if (!EffectSpecHandle.IsValid()) return;
		
		EffectSpecHandle.Data->SetSetByCallerMagnitude(GE_Tag, GE_Value);
	
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}
