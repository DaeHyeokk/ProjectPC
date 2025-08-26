// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"

#include "DataAsset/Player/PCDataAsset_PlayerAbilities.h"


void UPCPlayerAbilitySystemComponent::ApplyInitializedEffects()
{
	for (const TSubclassOf<UGameplayEffect>& GEClass : PlayerAbilityData->InitializedEffects)
	{
		// GetOwner()가 유효하지 않거나, 서버가 아니라면 return
		if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
		// GE SpecHandle 생성
		// MakeEffectContext() : 효과를 누가 어떤 상황에서 시전했는지 정보
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GEClass, 1, MakeEffectContext());

		// 자기 자신에게 GE 적용
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UPCPlayerAbilitySystemComponent::ApplyInitializedAbilities()
{
	return;
	// GetOwner()가 유효하지 않거나, 서버가 아니라면 return
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const TSubclassOf<UGameplayAbility>& IGAClass : PlayerAbilityData->InitializedAbilities)
	{
		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(IGAClass, 1, -1, nullptr);
		// 어빌리티 부여 및 한번만 실행 (인자 값을 참조로 받으므로 임시 객체 불가)
		GiveAbilityAndActivateOnce(Spec, nullptr);
	}

	for (const TPair<EPCShopRequestTypes, TSubclassOf<UGameplayAbility>>& SAClass : PlayerAbilityData->ShopAbility)
	{
		// 어빌리티 부여만 (인자 값을 상수 참조로 받으므로 임시 객체 가능)
		GiveAbility(FGameplayAbilitySpec(SAClass.Value, 0, static_cast<int32>(SAClass.Key), nullptr));
	}
}
