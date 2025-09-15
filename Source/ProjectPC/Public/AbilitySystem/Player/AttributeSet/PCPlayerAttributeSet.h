// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PCPlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel)
	FGameplayAttributeData PlayerLevel;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerXP)
	FGameplayAttributeData PlayerXP;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerGold)
	FGameplayAttributeData PlayerGold;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerHP)
	FGameplayAttributeData PlayerHP;

	UFUNCTION()
	void OnRep_PlayerLevel(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_PlayerXP(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_PlayerGold(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_PlayerHP(const FGameplayAttributeData& OldValue);

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	void CheckLevelUp();
	
public:
	ATTRIBUTE_ACCESSORS(UPCPlayerAttributeSet, PlayerLevel)
	ATTRIBUTE_ACCESSORS(UPCPlayerAttributeSet, PlayerXP)
	ATTRIBUTE_ACCESSORS(UPCPlayerAttributeSet, PlayerGold)
	ATTRIBUTE_ACCESSORS(UPCPlayerAttributeSet, PlayerHP)

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
