// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitPassiveGameplayAbility.h"

#include "AbilitySystemComponent.h"

void UPCUnitPassiveGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!ActorInfo || !ActorInfo->IsNetAuthority())
		return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
		return;

	if (FGameplayAbilitySpec* MutableSpec = ASC->FindAbilitySpecFromHandle(Spec.Handle))
	{
		if (!MutableSpec->IsActive())
		{
			ASC->TryActivateAbility(Spec.Handle);	
		}
	}
}

bool UPCUnitPassiveGameplayAbility::RollChance() const
{
	if (!bUseChance)
		return true;

	const float Percent = GetChancePercentForLevel(GetAbilityLevel());
	const float P = FMath::Clamp(Percent, 0.f, 100.f) * 0.01f;
	return FMath::FRand() <= P;
}

float UPCUnitPassiveGameplayAbility::GetChancePercentForLevel(int32 Level) const
{
	const int32 Idx = FMath::Max(1, Level) - 1;

	float ChancePercent = 100.f;
	
	for (int i=0; i<ChancePercentByLevel.Num(); ++i)
	{
		if (i <= Idx)
			ChancePercent = ChancePercentByLevel[Idx];
	}

	return ChancePercent;
}
