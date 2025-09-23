// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"
#include "PCCreepUnitCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCreepUnitCharacter : public APCBaseUnitCharacter
{
	GENERATED_BODY()
	
public:
	APCCreepUnitCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const override;
	virtual FGameplayTag GetUnitTypeTag() const override;

	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const override { return CreepUnitDataAsset; }
	virtual void SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset) override;
	
protected:
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UPCUnitAbilitySystemComponent> UnitAbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UPCUnitAttributeSet> UnitAttributeSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Data")
	TObjectPtr<UPCDataAsset_CreepUnitData> CreepUnitDataAsset;

	// 전투 관련 //
	virtual void OnDeathMontageCompleted() override;
};
