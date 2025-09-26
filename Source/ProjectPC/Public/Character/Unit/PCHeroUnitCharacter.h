// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "PCHeroUnitCharacter.generated.h"

class UPCHeroUnitAttributeSet;
class UPCHeroUnitAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCHeroUnitCharacter : public APCBaseUnitCharacter
{
	GENERATED_BODY()

public:
	APCHeroUnitCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPCHeroUnitAbilitySystemComponent* GetHeroUnitAbilitySystemComponent();
	const UPCHeroUnitAttributeSet* GetHeroUnitAttributeSet();
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const override;
	virtual FGameplayTag GetUnitTypeTag() const override;
	
	virtual bool HasLevelSystem() const override { return true; }
	virtual int32 GetUnitLevel() const override { return HeroLevel; };
	virtual void SetUnitLevel(const int32 Level) override;
	void LevelUp();
	
	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const override { return HeroUnitDataAsset; }
	virtual void SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset) override;

	void UpdateStatusBarUI() const;
	
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetJobSynergyTag() const;

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetSpeciesSynergyTag() const;
	
protected:
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UPCHeroUnitAbilitySystemComponent> HeroUnitAbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UPCHeroUnitAttributeSet> HeroUnitAttributeSet = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TObjectPtr<UPCDataAsset_HeroUnitData> HeroUnitDataAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_HeroLevel, meta=(ExposeOnSpawn=true), Category="Data")
	int32 HeroLevel = 1;

	UFUNCTION()
	virtual void OnRep_HeroLevel();

	// 전투 관련 //
	virtual void HandleGameStateChanged(const FGameplayTag NewStateTag) override;
	
public:
	virtual void ChangedOnTile(const bool IsOnField) override;
	
private:
	void RestoreFromCombatEnd();
	
public:
	UFUNCTION(BlueprintCallable, Category="DragAndDrop")
	void ActionDrag(const bool IsStart);
};