// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PCSynergyComponent.h"

#include "AbilitySystemComponent.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "DataAsset/Synergy/PCDataAsset_SynergyData.h"
#include "DataAsset/Synergy/PCDataAsset_SynergyDefinitionSet.h"
#include "Net/UnrealNetwork.h"
#include "Synergy/PCSynergyBase.h"

UPCSynergyComponent::UPCSynergyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

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
	if (GetOwner()->HasAuthority())
	{
		InitializeSynergyHandlersFromDefinitionSet();
		RecomputeAndReplicate();
	}
}

void UPCSynergyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner()->HasAuthority())
	{
		for (auto& KV : SynergyHandlers)
		{
			if (UPCSynergyBase* Synergy = KV.Value)
				Synergy->ResetAll();
		}
	}
	RegisterHeroSet.Empty();
	Super::EndPlay(EndPlayReason);
}

void UPCSynergyComponent::InitializeSynergyHandlersFromDefinitionSet()
{
	SynergyHandlers.Empty();

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
		if (SynergyHandlers.Contains(Key))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Synergy] Duplicate SynergyTag: %s. Skipping"), *Key.ToString());
			continue;
		}

		UPCSynergyBase* Handler = NewObject<UPCSynergyBase>(this, Def.SynergyClass);
		Handler->SetSynergyData(Def.SynergyData);

		SynergyHandlers.Add(Key, Handler);
	}
}


void UPCSynergyComponent::OnRep_SynergyCounts()
{
	DebugPrintSynergyCounts(true);
}

void UPCSynergyComponent::RegisterHero(APCHeroUnitCharacter* Hero)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Hero)
		return;
	
	RegisterHeroSet.Add(Hero);

	// FGameplayTagContainer Tags;
	// GetHeroSynergyTags(Hero, Tags);
	//
	// RecomputeForTags(Tags, true);
	
	RecomputeAndReplicate();
}

void UPCSynergyComponent::UnRegisterHero(APCHeroUnitCharacter* Hero)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Hero)
		return;

	// FGameplayTagContainer Tags;
	// GetHeroSynergyTags(Hero, Tags);
	//
	// RecomputeForTags(Tags, true);
	
	RegisterHeroSet.Remove(Hero);
	RecomputeAndReplicate();
}

void UPCSynergyComponent::RefreshHero(APCHeroUnitCharacter* Hero)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	FGameplayTagContainer Tags;
	GetHeroSynergyTags(Hero, Tags);

	//RecomputeForTags(Tags, true);
	
	RecomputeAndReplicate();
}

void UPCSynergyComponent::RebuildAll()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	// 유효하지 않은 포인터들 청소
	for (auto It = RegisterHeroSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	RecomputeAndReplicate();
}

void UPCSynergyComponent::GetSynergyCountMap(TMap<FGameplayTag, int32>& Out) const
{
	Out.Reset();
	for (const FSynergyCountEntry& E : SynergyCountArray.Entries)
	{
		if (E.Tag.IsValid())
		{
			Out.Add(E.Tag, E.Count);
		}
	}
}

void UPCSynergyComponent::RecomputeAndReplicate()
{
	TMap<FGameplayTag, FGameplayTagContainer> TypeToSynergyUnion;

	for (const TWeakObjectPtr<APCHeroUnitCharacter>& WeakHero : RegisterHeroSet)
	{
		const APCHeroUnitCharacter* Hero = WeakHero.Get();
		if (!Hero) continue;

		const FGameplayTag TypeTag = Hero->GetUnitTag();
		if (!TypeTag.IsValid()) continue;

		FGameplayTagContainer SynergyTags;
		GetHeroSynergyTags(Hero, SynergyTags);

		FGameplayTagContainer& Union = TypeToSynergyUnion.FindOrAdd(TypeTag);
		for (const FGameplayTag& Tag : SynergyTags)
		{
			Union.AddTag(Tag);
		}
	}

	TMap<FGameplayTag, int32> Acc;
	for (const auto& Pair : TypeToSynergyUnion)
	{
		const FGameplayTagContainer& Union = Pair.Value;
		for (const FGameplayTag& Tag : Union)
		{
			Acc.FindOrAdd(Tag) += 1;
		}
	}

	SynergyCountArray.ResetToMap(Acc);

	if (GetOwner()->HasAuthority())
	{
		ApplyAllSynergies(Acc);
	}
}

void UPCSynergyComponent::RecomputeForTags(const FGameplayTagContainer& AffectedTags, bool bForceReapplyUnits)
{
	TMap<FGameplayTag, int32> DeltaCounts;

	TArray<APCHeroUnitCharacter*> Heroes;
	GatherRegisteredHeroes(Heroes);

	TMap<FGameplayTag, TSet<FGameplayTag>> TagToUniqueTypeSet;

	for (APCHeroUnitCharacter* Hero : Heroes)
	{
		if (!Hero) continue;

		const FGameplayTag UnitTag = Hero->GetUnitTag();
		if (!UnitTag.IsValid())
			continue;

		FGameplayTagContainer OwnedSynergyTags;
		GetHeroSynergyTags(Hero, OwnedSynergyTags);
	}
}

void UPCSynergyComponent::ApplyForSynergyTag(const FGameplayTag& Tag, bool bForceReapplyUnits)
{
}

void UPCSynergyComponent::BindGameStateDelegates()
{
}

void UPCSynergyComponent::OnGameStateTagChanged(const FGameplayTag NewTag)
{
}

void UPCSynergyComponent::ApplyAllSynergies(const TMap<FGameplayTag, int32>& CountMap)
{
	TArray<APCHeroUnitCharacter*> CurrentHeroes;
	GatherRegisteredHeroes(CurrentHeroes);

	AActor* Instigator = GetOwner();

	for (auto& KV : SynergyHandlers)
	{
		const FGameplayTag SynergyTag = KV.Key;
		UPCSynergyBase* Handler = KV.Value;
		if (!Handler)
			continue;

		const int32* Found = CountMap.Find(SynergyTag);
		const int32 Count = Found ? *Found : 0;

		FSynergyApplyParams Params;
		Params.SynergyTag = SynergyTag;
		Params.Count = Count;
		Params.Units = CurrentHeroes;
		Params.Instigator = Instigator;

		Handler->Apply(Params);
	}
}

void UPCSynergyComponent::GetHeroSynergyTags(const APCHeroUnitCharacter* Hero, FGameplayTagContainer& OutSynergyTags) const
{
	OutSynergyTags.Reset();
	
	UAbilitySystemComponent* ASC = Hero ? Hero->GetAbilitySystemComponent() : nullptr;
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

void UPCSynergyComponent::GatherRegisteredHeroes(TArray<APCHeroUnitCharacter*>& OutHeroes) const
{
	OutHeroes.Reset();
	for (const TWeakObjectPtr<APCHeroUnitCharacter>& WeakHero : RegisterHeroSet)
	{
		if (APCHeroUnitCharacter* Hero = WeakHero.Get())
		{
			OutHeroes.Add(Hero);
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
