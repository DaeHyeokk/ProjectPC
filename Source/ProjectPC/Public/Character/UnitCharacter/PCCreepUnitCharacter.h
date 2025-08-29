// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"
#include "PCCreepUnitCharacter.generated.h"

class UPCDataAsset_CreepUnitData;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCreepUnitCharacter : public APCBaseUnitCharacter
{
	GENERATED_BODY()
	
public:
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const override;
	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const override;
	virtual FGameplayTag GetUnitTypeTag() const override;
	
protected:
	virtual TSubclassOf<UUserWidget> GetStatusBarClass() const override { return CreepStatusBarClass; }
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UPCUnitAbilitySystemComponent> UnitAbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, Category="Data")
	TObjectPtr<UPCDataAsset_CreepUnitData> CreepUnitDataAsset;

	UPROPERTY(EditAnywhere, Category="Data")
	TSubclassOf<UPCUnitStatusBarWidget> CreepStatusBarClass;
};
