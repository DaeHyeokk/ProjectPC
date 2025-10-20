// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Components/ActorComponent.h"
#include "Synergy/PCSynergyCountRep.h"
#include "PCSynergyComponent.generated.h"

class UPCSynergyBase;
class UPCDataAsset_SynergyDefinitionSet;
class APCHeroUnitCharacter;

USTRUCT()
struct FHeroSynergyTally
{
	GENERATED_BODY()
	
	TMap<FGameplayTag, int32> SynergyCountMap;

	void IncreaseSynergyTag(const FGameplayTag& SynergyTag, bool& OutIsUnique);
	void DecreaseSynergyTag(const FGameplayTag& SynergyTag, bool& OutIsRemoved);
	bool IsEmpty() const { return SynergyCountMap.IsEmpty(); }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCSynergyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPCSynergyComponent();

	void RegisterHero(APCHeroUnitCharacter* Hero);
	void UnRegisterHero(APCHeroUnitCharacter* Hero);
	
	const TArray<FSynergyCountEntry>& GetSynergyCountArray() const { return SynergyCountArray.Entries; };
	TArray<int32> GetSynergyThresholds(const FGameplayTag& SynergyTag) const;
	int32 GetSynergyTierIndexFromCount(const FGameplayTag& SynergyTag, int32 Count) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Synergy|Config", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPCDataAsset_SynergyDefinitionSet> SynergyDefinitionSet;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UPCSynergyBase>> SynergyToTagMap;
	
	TMap<FGameplayTag, int32> SynergyCountMap;
	
	TMap<FGameplayTag, int32> HeroTagCountMap;
	TMap<FGameplayTag, FHeroSynergyTally> HeroSynergyMap;
	TSet<TWeakObjectPtr<APCHeroUnitCharacter>> RegisterHeroSet;
	
	UPROPERTY(ReplicatedUsing=OnRep_SynergyCountArray)
	FSynergyCountArray SynergyCountArray;

	
	UFUNCTION()
	void OnRep_SynergyCountArray();
	
	void InitializeSynergyHandlersFromDefinitionSet();

	void UpdateSynergyCountMap(const FGameplayTagContainer& SynergyTags, const bool bRegisterHero);
	void ApplySynergyEffects(const FGameplayTag& SynergyTag);
	
	void GetHeroSynergyTags(const APCHeroUnitCharacter* Hero, FGameplayTagContainer& OutSynergyTags) const;
	void GatherRegisteredHeroes(TArray<APCHeroUnitCharacter*>& OutHeroes);

	FDelegateHandle GameStateDelegateHandle;
	
	void BindGameStateDelegates();
	void OnGameStateTagChanged(const FGameplayTag& NewTag);

	void OnCombatActiveAction();
	void OnCombatEndAction();
	
	void OnHeroDestroyed(APCHeroUnitCharacter* DestroyedHero);
	void OnHeroSynergyTagChanged(const APCHeroUnitCharacter* Hero, const FGameplayTag& SynergyTag, bool bIsAdded);
	
	// 디버그용
public:
	UFUNCTION(BlueprintCallable, Category="Synergy|Debug")
	void DebugPrintSynergyCounts(bool bAlsoOnScreen = true) const;
};
