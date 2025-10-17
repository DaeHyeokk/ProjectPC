// Fill out your copyright notice in the Description page of Project Settings.


#include "Synergy/PCSynergyBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "DataAsset/Synergy/PCDataAsset_SynergyData.h"


void UPCSynergyBase::Apply(const FSynergyApplyParams& Params)
{
	if (!SynergyData || !SynergyData->IsValidData())
		return;
	if (!Params.Instigator || !Params.Instigator->HasAuthority())
		return;

	const int32 DefaultTier = SynergyData->ComputeActiveTierIndex(Params.Count);
	const int32 NewTierIdx = ComputeActiveTierIndex(Params, DefaultTier);

	// 티어 변동 없고 유닛 재적용 강제 플래그도 false면 스킵
	if (NewTierIdx == CachedTierIndex && !Params.bForceReapplyUnits)
		return;

	OnBeforeApply(Params, NewTierIdx);

	// 기존 부여했던 능력 해제 후 새로운 티어 부여
	RevokeAllGrantGAs();
	RevokeAllGrantGEs();
	
	if (NewTierIdx >= 0)
	{
		if (!SynergyData->GetGrantGAs().IsEmpty())
			GrantTier(Params, NewTierIdx);
		
		if (!SynergyData->GetApplyEffects().IsEmpty())
			ApplyTierEffects(Params, NewTierIdx);
	}
	
	CachedTierIndex = NewTierIdx;
	OnAfterApply(Params, NewTierIdx);
}

void UPCSynergyBase::ResetGrantGAs()
{
	RevokeAllGrantGAs();
}

void UPCSynergyBase::ResetGrantGEs()
{
	RevokeAllGrantGEs();
}

void UPCSynergyBase::ResetAll()
{
	RevokeAllGrantGAs();
	RevokeAllGrantGEs();
	CachedTierIndex = -1;
}

void UPCSynergyBase::SelectRecipients(const FSynergyApplyParams& Params, const struct FSynergyTier& Tier,
                                      TArray<APCHeroUnitCharacter*>& OutRecipients) const
{
	OutRecipients.Reset();

	switch (Tier.RecipientPolicy)
	{
		// 시너지 보유 유닛 전부
	case ESynergyRecipientPolicy::AllOwners:
		{
			for (APCHeroUnitCharacter* Hero : Params.Units)
			{
				const UAbilitySystemComponent* ASC = Hero ? Hero->GetAbilitySystemComponent() : nullptr;
				if (ASC && ASC->HasMatchingGameplayTag(SynergyData->GetSynergyTag()))
					OutRecipients.Add(Hero);
			}
			break;
		}
		// 시너지를 보유한 유닛 중 랜덤
	case ESynergyRecipientPolicy::RandomAmongOwners:
		{
			// 시너지를 보유한 유닛만 선별
			TArray<APCHeroUnitCharacter*> Owners;
			for (APCHeroUnitCharacter* Hero : Params.Units)
			{
				const UAbilitySystemComponent* ASC = Hero ? Hero->GetAbilitySystemComponent() : nullptr;
				if (ASC && ASC->HasMatchingGameplayTag(SynergyData->GetSynergyTag()))
					Owners.Add(Hero);
			}
			// 선별한 유닛들 랜덤으로 섞고 앞에서부터 뽑음
			for (int32 i=0; i<Owners.Num(); ++i)
			{
				const int32 SwapIdx = FMath::RandRange(i, Owners.Num()-1);
				if (i != SwapIdx) Owners.Swap(i, SwapIdx);
			}
			const int32 PickCnt = FMath::Clamp(Tier.RandomPickCount, 1, Owners.Num());
			for (int32 i=0; i<PickCnt; ++i)
				OutRecipients.Add(Owners[i]);
			break;
		}

	case ESynergyRecipientPolicy::RandomAmongAllies:
		{
			TArray<APCHeroUnitCharacter*> Allies = Params.Units;

			// 유닛들 랜덤으로 섞고 앞에서부터 뽑음
			for (int32 i=0; i<Allies.Num(); ++i)
			{
				const int32 SwapIdx = FMath::RandRange(i, Allies.Num()-1);
				if (i != SwapIdx) Allies.Swap(i, SwapIdx);
			}
			const int32 PickCnt = FMath::Clamp(Tier.RandomPickCount, 1, Allies.Num());
			for (int32 i=0; i<PickCnt; ++i)
				OutRecipients.Add(Allies[i]);
			break;
		}
		
		// 모든 아군
	case ESynergyRecipientPolicy::AllAllies:
	default:
		OutRecipients = Params.Units;
		break;
	}
}

void UPCSynergyBase::RevokeAllGrantGAs()
{
	for (auto& KV : ActiveGrantGAs)
	{
		if (UAbilitySystemComponent* ASC = KV.Key.Get())
		{
			for (const FGameplayAbilitySpecHandle& Handle : KV.Value)
			{
				if (Handle.IsValid())
					ASC->ClearAbility(Handle);
			}
		}
	}
	ActiveGrantGAs.Reset();
}

void UPCSynergyBase::RevokeAllGrantGEs()
{
	for (auto& KV : ActiveGrantGEs)
	{
		if (UAbilitySystemComponent* ASC = KV.Key.Get())
		{
			for (const FActiveGameplayEffectHandle& Handle : KV.Value)
			{
				if (Handle.IsValid())
					ASC->RemoveActiveGameplayEffect(Handle);
			}
		}
	}
	ActiveGrantGEs.Reset();
}

void UPCSynergyBase::GrantTier(const FSynergyApplyParams& Params, int32 TierIndex)
{
	const FSynergyTier* Tier = SynergyData->GetTier(TierIndex);
	if (!Tier)
		return;

	TArray<APCHeroUnitCharacter*> Recipients;
	SelectRecipients(Params, *Tier, Recipients);
	if (Recipients.Num() == 0)
		return;

	for (APCHeroUnitCharacter* Hero : Recipients)
	{
		if (!Hero) continue;
		if (UAbilitySystemComponent* HeroASC = Hero->GetAbilitySystemComponent())
		{
			TArray<FGameplayAbilitySpecHandle>& Handles = ActiveGrantGAs.FindOrAdd(HeroASC);

			for (const FSynergyGrantGA& GrantGA : SynergyData->GetGrantGAs())
			{
				if (!GrantGA.IsValid()) continue;

				const int32 Level = TierIndex + 1;
				UObject* SourceObject = SynergyData->GetUnitAbilityConfig();

				FGameplayAbilitySpec Spec(GrantGA.AbilityClass, Level, INDEX_NONE, SourceObject);

				const FGameplayAbilitySpecHandle Handle = HeroASC->GiveAbility(Spec);
				if (Handle.IsValid())
					Handles.Add(Handle);
			}
		}
	}
}

void UPCSynergyBase::ApplyTierEffects(const FSynergyApplyParams& Params, int32 TierIndex)
{
	const FSynergyTier* Tier = SynergyData->GetTier(TierIndex);
	if (!Tier)
		return;
	
	TArray<APCHeroUnitCharacter*> Recipients;
	SelectRecipients(Params, *Tier, Recipients);
	if (Recipients.Num() == 0) return;

	for (APCHeroUnitCharacter* Hero : Recipients)
	{
		if (!Hero) continue;

		if (UAbilitySystemComponent* HeroASC = Hero->GetAbilitySystemComponent())
		{
			TArray<FActiveGameplayEffectHandle>& Handles = ActiveGrantGEs.FindOrAdd(HeroASC);

			for (const auto& EffectSpec : SynergyData->GetApplyEffects().EffectSpecs)
			{
				if (!EffectSpec) continue;

				const int32 Level = TierIndex + 1;
				const FActiveGameplayEffectHandle Handle = EffectSpec->ApplyEffectSelf(HeroASC, Level);
				
				if (Handle.IsValid())
					Handles.Add(Handle);
			}
		}
	}
}