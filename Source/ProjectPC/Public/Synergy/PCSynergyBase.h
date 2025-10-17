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

	UPROPERTY()
	bool bForceReapplyUnits = false;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECTPC_API UPCSynergyBase : public UObject
{
	GENERATED_BODY()

public:
	void Apply(const FSynergyApplyParams& Params);
	void ResetGrantGAs();
	void ResetGrantGEs();
	void ResetAll();

	FORCEINLINE void SetSynergyData(UPCDataAsset_SynergyData* InSynergyData) { SynergyData = InSynergyData; }
	FORCEINLINE const UPCDataAsset_SynergyData* GetSynergyData() const { return SynergyData; }
	
protected:
	// 기본적인 카운팅 룰을 따르지 않는 시너지들이 있을 경우 하위클래스로 파생해서 오버라이드
	virtual int32 ComputeActiveTierIndex(const FSynergyApplyParams& Params, int32 DefaultIndex) const { return DefaultIndex; }

	virtual void SelectRecipients(const FSynergyApplyParams& Params, const struct FSynergyTier& Tier,
		TArray<APCHeroUnitCharacter*>& OutRecipients) const;

	// 추후에 이벤트 받아서 이펙트 연출할거면 사용
	virtual void OnBeforeApply(const FSynergyApplyParams& Params, int32 NewTierIndex) {}
	virtual void OnAfterApply(const FSynergyApplyParams& Params, int32 NewTierIndex) {}

private:
	UPROPERTY(EditAnywhere, Category="Synergy")
	TObjectPtr<UPCDataAsset_SynergyData> SynergyData = nullptr;
	
	int32 CachedTierIndex = -1;

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FGameplayAbilitySpecHandle>> ActiveGrantGAs;
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveGrantGEs;

	void RevokeAllGrantGAs();
	void RevokeAllGrantGEs();
	void GrantTier(const FSynergyApplyParams& Params, int32 TierIndex);
	void ApplyTierEffects(const FSynergyApplyParams& Params, int32 TierIndex);
};
