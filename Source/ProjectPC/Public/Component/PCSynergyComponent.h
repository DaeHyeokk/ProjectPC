// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Synergy/PCSynergyCountRep.h"
#include "PCSynergyComponent.generated.h"


class UPCSynergyBase;
class UPCDataAsset_SynergyDefinitionSet;
class APCHeroUnitCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCSynergyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPCSynergyComponent();

	void RegisterHero(APCHeroUnitCharacter* Hero);
	void UnRegisterHero(APCHeroUnitCharacter* Hero);
	void RefreshHero(APCHeroUnitCharacter* Hero);
	void RebuildAll();

	const FSynergyCountArray& GetSynergyCountArray() const { return SynergyCountArray; }
	void GetSynergyCountMap(TMap<FGameplayTag, int32>& Out) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category="Synergy|Config")
	TObjectPtr<UPCDataAsset_SynergyDefinitionSet> SynergyDefinitionSet;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UPCSynergyBase>> SynergyHandlers;
	
	UPROPERTY(ReplicatedUsing=OnRep_SynergyCounts)
	FSynergyCountArray SynergyCountArray;
	UFUNCTION()
	void OnRep_SynergyCounts();
	
private:
	TSet<TWeakObjectPtr<APCHeroUnitCharacter>> RegisterHeroSet;

	void InitializeSynergyHandlersFromDefinitionSet();
	void RecomputeAndReplicate();

	void RecomputeForTags(const FGameplayTagContainer& AffectedTags, bool bForceReapplyUnits);
	void ApplyForSynergyTag(const FGameplayTag& Tag, bool bForceReapplyUnits);
	
	void ApplyAllSynergies(const TMap<FGameplayTag, int32>& CountMap);
	
	void GetHeroSynergyTags(const APCHeroUnitCharacter* Hero, FGameplayTagContainer& OutSynergyTags) const;
	void GatherRegisteredHeroes(TArray<APCHeroUnitCharacter*>& OutHeroes) const;

	void BindGameStateDelegates();
	void OnGameStateTagChanged(const FGameplayTag NewTag);
	
	// 디버그용
public:
	UFUNCTION(BlueprintCallable, Category="Synergy|Debug")
	void DebugPrintSynergyCounts(bool bAlsoOnScreen = true) const;
};
