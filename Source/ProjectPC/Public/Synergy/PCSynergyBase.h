// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "PCSynergyBase.generated.h"

class UAbilitySystemComponent;
class UPCDataAsset_SynergyData;
class APCHeroUnitCharacter;
/**
 * 
 */
USTRUCT()
struct FSynergyApplyParams
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SynergyTag;

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	TArray<APCHeroUnitCharacter*> Units;

	UPROPERTY()
	AActor* Instigator = nullptr;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECTPC_API UPCSynergyBase : public UObject
{
	GENERATED_BODY()

public:
	void GrantGE(const FSynergyApplyParams& Params);
	void CombatActiveGrant(const FSynergyApplyParams& Params);
	void CombatEndRevoke();
	void RevokeAllGrantGAs();
	void RevokeAllGrantGEs();
	void RevokeHeroGrantGEs(const APCHeroUnitCharacter* Hero);
	void RevokeHeroGrantGAs(const APCHeroUnitCharacter* Hero);
	void ResetAll();

	void PlayActiveParticleAtUnit(const FSynergyApplyParams& Params) const;
	
	FORCEINLINE void SetSynergyData(UPCDataAsset_SynergyData* InSynergyData) { SynergyData = InSynergyData; }
	FORCEINLINE const UPCDataAsset_SynergyData* GetSynergyData() const { return SynergyData; }
	
protected:
	// 기본적인 카운팅 룰을 따르지 않는 시너지들이 있을 경우 하위클래스로 파생해서 오버라이드
	virtual int32 ComputeActiveTierIndex(const FSynergyApplyParams& Params, int32 DefaultIndex) const { return DefaultIndex; }

	virtual void SelectRecipients(const FSynergyApplyParams& Params, const struct FSynergyTier& Tier,
		TArray<APCHeroUnitCharacter*>& OutRecipients) const;

private:
	UPROPERTY(EditAnywhere, Category="Synergy")
	TObjectPtr<UPCDataAsset_SynergyData> SynergyData = nullptr;
	
	int32 CachedTierIndex = -1;

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FGameplayAbilitySpecHandle>> ActiveGrantGAs;
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveGrantGEs;
	
	void GrantAbility(const FSynergyApplyParams& Params, const FSynergyTier* Tier, int32 Level);
	void GrantEffects(const FSynergyApplyParams& Params, const FSynergyTier* Tier, int32 Level);

	bool IsRandomAmongPolicy(const FSynergyTier* SynergyTier) const;
};
