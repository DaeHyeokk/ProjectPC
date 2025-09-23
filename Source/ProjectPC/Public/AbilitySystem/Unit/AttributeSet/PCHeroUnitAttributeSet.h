// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "PCHeroUnitAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroUnitAttributeSet : public UPCUnitAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	ATTRIBUTE_ACCESSORS(ThisClass, MaxMana);
	ATTRIBUTE_ACCESSORS(ThisClass, CurrentMana);
	ATTRIBUTE_ACCESSORS(ThisClass, UltimateDamage);
	ATTRIBUTE_ACCESSORS(ThisClass, UltimateCost);
	ATTRIBUTE_ACCESSORS(ThisClass, PhysicalDamageMultiplier);
	ATTRIBUTE_ACCESSORS(ThisClass, MagicDamageMultiplier);
	ATTRIBUTE_ACCESSORS(ThisClass, CritChance);
	ATTRIBUTE_ACCESSORS(ThisClass, CritMultiplier);
	ATTRIBUTE_ACCESSORS(ThisClass, LifeSteal);
	ATTRIBUTE_ACCESSORS(ThisClass, SpellVamp);

protected:
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_CurrentMana)
	FGameplayAttributeData CurrentMana;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_UltimateDamage)
	FGameplayAttributeData UltimateDamage;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_UltimateCost)
	FGameplayAttributeData UltimateCost;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_PhysicalDamageMultiplier)
	FGameplayAttributeData PhysicalDamageMultiplier;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_MagicDamageMultiplier)
	FGameplayAttributeData MagicDamageMultiplier;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_CritChance)
	FGameplayAttributeData CritChance;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_CritMultiplier)
	FGameplayAttributeData CritMultiplier;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_LifeSteal)
	FGameplayAttributeData LifeSteal;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_SpellVamp)
	FGameplayAttributeData SpellVamp;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);
	UFUNCTION()
	void OnRep_CurrentMana(const FGameplayAttributeData& OldCurrentMana);

	UFUNCTION()
	void OnRep_UltimateDamage(const FGameplayAttributeData& OldUltimateDamage);
	UFUNCTION()
	void OnRep_UltimateCost(const FGameplayAttributeData& OldUltimateCost);

	UFUNCTION()
	void OnRep_PhysicalDamageMultiplier(const FGameplayAttributeData& OldPhysicalDamageMultiplier);

	UFUNCTION()
	void OnRep_MagicDamageMultiplier(const FGameplayAttributeData& OldMagicDamageMultiplier);

	UFUNCTION()
	void OnRep_CritChance(const FGameplayAttributeData& OldCritChance);

	UFUNCTION()
	void OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier);
	
	UFUNCTION()
	void OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal);
	
	UFUNCTION()
	void OnRep_SpellVamp(const FGameplayAttributeData& OldSpellVamp);
};
