// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PCUnitAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)		\
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)	\
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)				\
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)				\
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPCUnitAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, CurrentHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, BaseDamage);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackRange);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackSpeedIncreaseMultiplier);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackSpeedDecreaseMultiplier);
	ATTRIBUTE_ACCESSORS(ThisClass, PhysicalDefense);
	ATTRIBUTE_ACCESSORS(ThisClass, MagicDefense);
	
	// 시너지로 증감되는 속성들 //
	ATTRIBUTE_ACCESSORS(ThisClass, FlatDamageBlock);
	ATTRIBUTE_ACCESSORS(ThisClass, EvasionChance);
	
	float GetEffectiveAttackSpeed() const;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes",  ReplicatedUsing=OnRep_CurrentHealth)
	FGameplayAttributeData CurrentHealth;	
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_BaseDamage)
	FGameplayAttributeData BaseDamage;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_AttackRange)
	FGameplayAttributeData AttackRange;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_AttackSpeedIncreaseMultiplier)
	FGameplayAttributeData AttackSpeedIncreaseMultiplier;
	
	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_AttackSpeedDecreaseMultiplier)
	FGameplayAttributeData AttackSpeedDecreaseMultiplier;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_PhysicalDefense)
	FGameplayAttributeData PhysicalDefense;

	UPROPERTY(BlueprintReadOnly, Category="Unit Attributes", ReplicatedUsing=OnRep_MagicDefense)
	FGameplayAttributeData MagicDefense;

	UPROPERTY(BlueprintReadOnly, Category="Synergy|Defense")
	FGameplayAttributeData FlatDamageBlock;
	
	UPROPERTY(BlueprintReadOnly, Category="Synergy|Defense")
	FGameplayAttributeData EvasionChance;
	
	UFUNCTION()
	void AdjustAttributeForMaxChange(
		const FGameplayAttributeData& AffectedAttribute,
		const FGameplayAttributeData& MaxAttribute,
		float NewMaxValue,
		const FGameplayAttribute& AffectedAttributeProperty
		) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	void OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth);
	
	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldBaseDamage);
	
	UFUNCTION()
	void OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange);
	
	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);
	UFUNCTION()
	void OnRep_AttackSpeedIncreaseMultiplier(const FGameplayAttributeData& OldAttackSpeedIncreaseMultiplier);
	UFUNCTION()
	void OnRep_AttackSpeedDecreaseMultiplier(const FGameplayAttributeData& OldAttackSpeedDecreaseMultiplier);
	
	UFUNCTION()
	void OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense);
	
	UFUNCTION()
	void OnRep_MagicDefense(const FGameplayAttributeData& OldMagicDefense);
};