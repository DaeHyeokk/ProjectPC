// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PCSynergyComponent.h"

#include "AbilitySystemComponent.h"
#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "DataAsset/Synergy/PCDataAsset_SynergyData.h"
#include "DataAsset/Synergy/PCDataAsset_SynergyDefinitionSet.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Net/UnrealNetwork.h"
#include "Synergy/PCSynergyBase.h"

void FHeroSynergyTally::IncreaseSynergyTag(const FGameplayTag& SynergyTag, bool& OutIsUnique)
{
	int32& Count = SynergyCountMap.FindOrAdd(SynergyTag);
	Count++;

	if (Count == 1)
		OutIsUnique = true;
}

void FHeroSynergyTally::DecreaseSynergyTag(const FGameplayTag& SynergyTag, bool& OutIsRemoved)
{
	int32& Count = SynergyCountMap.FindOrAdd(SynergyTag);
	Count--;

	if (Count <= 0)
	{
		SynergyCountMap.Remove(SynergyTag);
		OutIsRemoved = true;
	}
}

UPCSynergyComponent::UPCSynergyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SynergyCountMap.Reset();
	SetIsReplicatedByDefault(true);
}

void UPCSynergyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPCSynergyComponent, SynergyCountArray);
}

void UPCSynergyComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSynergyHandlersFromDefinitionSet();
	
	if (GetOwner()->HasAuthority())
	{
		BindGameStateDelegates();
	}
}

void UPCSynergyComponent::InitializeSynergyHandlersFromDefinitionSet()
{
	SynergyToTagMap.Empty();

	if (!SynergyDefinitionSet)
		return;

	for (const FSynergyDefinition& Def : SynergyDefinitionSet->Definitions)
	{
		if (!Def.SynergyClass || !Def.SynergyData)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Synergy] Invalid Definition (class/data missing)"));
			continue;
		}

		const FGameplayTag Key = Def.SynergyData->GetSynergyTag();
		if (!Key.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Synergy] Definition has InValid SynergyTag"));
			continue;
		}
		if (SynergyToTagMap.Contains(Key))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Synergy] Duplicate SynergyTag: %s. Skipping"), *Key.ToString());
			continue;
		}

		UPCSynergyBase* SynergyBase = NewObject<UPCSynergyBase>(this, Def.SynergyClass);
		SynergyBase->SetSynergyData(Def.SynergyData);

		SynergyToTagMap.Add(Key, SynergyBase);
	}
}


void UPCSynergyComponent::OnRep_SynergyCountArray()
{
	DebugPrintSynergyCounts(true);
}

void UPCSynergyComponent::RegisterHero(APCHeroUnitCharacter* Hero)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Hero)
		return;

	if (!RegisterHeroSet.Contains(Hero))
	{
		RegisterHeroSet.Add(Hero);
		Hero->OnHeroDestroyed.AddUObject(this, &ThisClass::OnHeroDestroyed);
		Hero->OnHeroSynergyTagChanged.AddUObject(this, &ThisClass::OnHeroSynergyTagChanged);

		FGameplayTagContainer SynergyTags;
		GetHeroSynergyTags(Hero, SynergyTags);

		const FGameplayTag HeroTag = Hero->GetUnitTag();
		FHeroSynergyTally& SynergyTally = HeroSynergyMap.FindOrAdd(HeroTag);
		FGameplayTagContainer NewSynergyTags;
		
		for (const FGameplayTag& SynergyTag : SynergyTags)
		{
			bool bIsUnique = false;
			SynergyTally.IncreaseSynergyTag(SynergyTag, bIsUnique);
			
			if (bIsUnique)
				NewSynergyTags.AddTag(SynergyTag);
		}
		
		if (!NewSynergyTags.IsEmpty())
		{
			UpdateSynergyCountMap(NewSynergyTags, true);
		}

		for (const FGameplayTag& SynergyTag : SynergyTags)
		{
			ApplySynergyEffects(SynergyTag);
		}
	}
}

void UPCSynergyComponent::UnRegisterHero(APCHeroUnitCharacter* Hero)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Hero)
		return;

	if (RegisterHeroSet.Contains(Hero))
	{
		RegisterHeroSet.Remove(Hero);
		Hero->OnHeroDestroyed.RemoveAll(this);
		Hero->OnHeroSynergyTagChanged.RemoveAll(this);
		
		FGameplayTagContainer SynergyTags;
		GetHeroSynergyTags(Hero, SynergyTags);

		const FGameplayTag HeroTag = Hero->GetUnitTag();
		FHeroSynergyTally& SynergyTally = HeroSynergyMap.FindOrAdd(HeroTag);
		FGameplayTagContainer RemoveSynergyTags;

		for (const FGameplayTag& SynergyTag : SynergyTags)
		{
			if (UPCSynergyBase* Synergy = *SynergyToTagMap.Find(SynergyTag))
			{
				Synergy->RevokeHeroGrantGEs(Hero);
				Synergy->RevokeHeroGrantGAs(Hero);
			}
			
			bool bIsRemoved = false;
			SynergyTally.DecreaseSynergyTag(SynergyTag, bIsRemoved);

			if (bIsRemoved)
				RemoveSynergyTags.AddTag(SynergyTag);
		}
		
		if (!RemoveSynergyTags.IsEmpty())
		{
			UpdateSynergyCountMap(RemoveSynergyTags, false);
		}

		if (SynergyTally.IsEmpty())
			HeroSynergyMap.Remove(HeroTag);

		for (const FGameplayTag& SynergyTag : SynergyTags)
		{
			ApplySynergyEffects(SynergyTag);
		}
	}
}

TArray<int32> UPCSynergyComponent::GetSynergyThresholds(const FGameplayTag& SynergyTag) const
{
	TArray<int32> Result;

	if (const UPCSynergyBase* Synergy = SynergyToTagMap.FindRef(SynergyTag))
	{
		const UPCDataAsset_SynergyData* SynergyData = Synergy->GetSynergyData();
		for (const FSynergyTier& SynergyTier : SynergyData->GetAllTiers())
		{
			Result.Add(SynergyTier.Threshold);
		}
	}

	return Result;
}

int32 UPCSynergyComponent::GetSynergyTierIndexFromCount(const FGameplayTag& SynergyTag, int32 Count) const
{
	int32 Result = -1;
	
	if (const UPCSynergyBase* Synergy = SynergyToTagMap.FindRef(SynergyTag))
	{
		if (const UPCDataAsset_SynergyData* SynergyData = Synergy->GetSynergyData())
		{
			Result = SynergyData->ComputeActiveTierIndex(Count);
		}
	}

	return Result;
}

void UPCSynergyComponent::UpdateSynergyCountMap(const FGameplayTagContainer& SynergyTags, const bool bRegisterHero)
{
	for (const FGameplayTag& SynergyTag : SynergyTags)
	{
		int32& SynergyCnt = SynergyCountMap.FindOrAdd(SynergyTag);
		if (bRegisterHero)
		{
			SynergyCnt++;	
		}
		else
		{
			SynergyCnt--;
			if (SynergyCnt <= 0)
			{
				SynergyCountMap.Remove(SynergyTag);
			}
		}
	}

	SynergyCountArray.ResetToMap(SynergyCountMap);
}

void UPCSynergyComponent::ApplySynergyEffects(const FGameplayTag& SynergyTag)
{
	UPCSynergyBase* Synergy = *SynergyToTagMap.Find(SynergyTag);
	
	if (!Synergy)
		return;
	
	TArray<APCHeroUnitCharacter*> CurrentHeroes;
	GatherRegisteredHeroes(CurrentHeroes);

	AActor* Instigator = GetOwner();
	int32 Count = SynergyCountMap.FindRef(SynergyTag);
	
	FSynergyApplyParams Params;
	Params.SynergyTag = SynergyTag;
	Params.Count = Count;
	Params.Units = CurrentHeroes;
	Params.Instigator = Instigator;
		
	Synergy->GrantGE(Params);
}

void UPCSynergyComponent::BindGameStateDelegates()
{
	if (GetOwner()->HasAuthority())
	{
		if (APCCombatGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr)
		{
			GS->OnGameStateTagChanged.AddUObject(this, &ThisClass::OnGameStateTagChanged);
		}
	}
}

void UPCSynergyComponent::OnGameStateTagChanged(const FGameplayTag& NewTag)
{
	if (GetOwner()->HasAuthority())
	{
		if (NewTag.MatchesTag(GameStateTags::Game_State_Combat_Active))
		{
			OnCombatActiveAction();
		}
		else
		{
			OnCombatEndAction();
		}
	}
}

void UPCSynergyComponent::OnCombatActiveAction()
{
	TArray<APCHeroUnitCharacter*> CurrentHeroes;
	GatherRegisteredHeroes(CurrentHeroes);

	AActor* Instigator = GetOwner();

	for (auto& KV : SynergyToTagMap)
	{
		const FGameplayTag SynergyTag = KV.Key;
		UPCSynergyBase* Handler = KV.Value;
		if (!Handler)
			continue;

		const int32* Found = SynergyCountMap.Find(SynergyTag);
		const int32 Count = Found ? *Found : 0;

		FSynergyApplyParams Params;
		Params.SynergyTag = SynergyTag;
		Params.Count = Count;
		Params.Units = CurrentHeroes;
		Params.Instigator = Instigator;
		
		Handler->CombatActiveGrant(Params);
	}
}

void UPCSynergyComponent::OnCombatEndAction()
{
	for (auto& KV : SynergyToTagMap)
	{
		UPCSynergyBase* Handler = KV.Value;
		if (!Handler)
			continue;
		
		Handler->CombatEndRevoke();
	}
}

void UPCSynergyComponent::OnHeroDestroyed(APCHeroUnitCharacter* DestroyedHero)
{
	if (RegisterHeroSet.Contains(DestroyedHero))
	{
		UnRegisterHero(DestroyedHero);
	}
}

void UPCSynergyComponent::OnHeroSynergyTagChanged(const APCHeroUnitCharacter* Hero, const FGameplayTag& SynergyTag,
                                                  bool bIsAdded)
{
	if (Hero)
	{
		const FGameplayTag HeroTag = Hero->GetUnitTag();
		FHeroSynergyTally& SynergyTally = HeroSynergyMap.FindOrAdd(HeroTag);
		bool bIsUniqueOrRemoved = false;
		
		if (bIsAdded)
		{
			SynergyTally.IncreaseSynergyTag(SynergyTag, bIsUniqueOrRemoved);
			if (bIsUniqueOrRemoved)
				UpdateSynergyCountMap(FGameplayTagContainer(SynergyTag), true);
		}
		else
		{
			SynergyTally.DecreaseSynergyTag(SynergyTag, bIsUniqueOrRemoved);
			if (bIsUniqueOrRemoved)
			{
				UpdateSynergyCountMap(FGameplayTagContainer(SynergyTag), true);
			}
			if (SynergyTally.IsEmpty())
				HeroSynergyMap.Remove(HeroTag);
		}
	}
}

void UPCSynergyComponent::GetHeroSynergyTags(const APCHeroUnitCharacter* Hero, FGameplayTagContainer& OutSynergyTags) const
{
	OutSynergyTags.Reset();
	
	if (UAbilitySystemComponent* ASC = Hero ? Hero->GetAbilitySystemComponent() : nullptr)
	{
		FGameplayTagContainer Owned;
		ASC->GetOwnedGameplayTags(Owned);
		for (const FGameplayTag& Tag : Owned)
		{
			if (Tag.MatchesTag(SynergyGameplayTags::Synergy))
			{
				OutSynergyTags.AddTag(Tag);
			}
		}
	}
}

void UPCSynergyComponent::GatherRegisteredHeroes(TArray<APCHeroUnitCharacter*>& OutHeroes)
{
	OutHeroes.Reset();
	
	for (auto It = RegisterHeroSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
		else
		{
			OutHeroes.Add(It->Get());
		}
	}
}

void UPCSynergyComponent::DebugPrintSynergyCounts(bool bAlsoOnScreen) const
{
	// 1) 헤더 라인
	UE_LOG(LogTemp, Log, TEXT("=== [Synergy] ReplicatedCounts (%d entries) ==="),
		SynergyCountArray.Entries.Num());

	// 2) 비었으면 안내
	if (SynergyCountArray.Entries.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("(empty)"));
		if (bAlsoOnScreen && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				/*Key=*/-1, /*Time=*/2.0f, /*Color=*/FColor::Yellow,
				TEXT("[Synergy] (empty)"));
		}
		return;
	}

	// 3) 항목들 출력
	for (const FSynergyCountEntry& E : SynergyCountArray.Entries)
	{
		const FString TagStr = E.Tag.IsValid() ? E.Tag.ToString() : TEXT("InvalidTag");
		UE_LOG(LogTemp, Log, TEXT("  %s = %d"), *TagStr, E.Count);

		if (bAlsoOnScreen && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				/*Key=*/-1, /*Time=*/2.0f, /*Color=*/FColor::Cyan,
				FString::Printf(TEXT("[Synergy] %s = %d"), *TagStr, E.Count));
		}
	}
}
