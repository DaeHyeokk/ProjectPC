// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"
#include "UI/Unit/PCHeroStatusBarWidget.h"
#include "PCHeroUnitCharacter.generated.h"

class UPCDataAsset_HeroUnitData;
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

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPCHeroUnitAbilitySystemComponent* GetHeroUnitAbilitySystemComponent();
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const override;
	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const override;
	virtual FGameplayTag GetUnitTypeTag() const override;
	
	virtual bool HasLevelSystem() const override { return true; }
	virtual int32 GetUnitLevel() const override { return HeroLevel; };

	virtual TSubclassOf<UUserWidget> GetStatusBarClass() const override { return HeroStatusBarClass; }
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget) override;
	
	UFUNCTION(BlueprintCallable)
	void LevelUp();

	void UpdateStatusBarUI() const;
	
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetJobSynergyTag() const;

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetSpeciesSynergyTag() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UPCHeroUnitAbilitySystemComponent> HeroUnitAbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category="Data")
	TObjectPtr<UPCDataAsset_HeroUnitData> HeroUnitDataAsset;

	UPROPERTY(EditAnywhere, Category="Data")
	TSubclassOf<UPCHeroStatusBarWidget> HeroStatusBarClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_HeroLevel, meta=(ExposeOnSpawn=true), Category="Data")
	int32 HeroLevel = 1;

	UFUNCTION()
	void OnRep_HeroLevel();
};
