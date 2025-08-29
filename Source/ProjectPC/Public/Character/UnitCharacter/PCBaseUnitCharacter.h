// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCBaseUnitCharacter.generated.h"

class UWidgetComponent;
class UGameplayAbility;
class UPCUnitAttributeSet;
class UPCUnitAbilitySystemComponent;

UCLASS()
class PROJECTPC_API APCBaseUnitCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APCBaseUnitCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const;
	const UPCUnitAttributeSet* GetUnitAttributeSet() const;
	UPCDataAsset_UnitAnimSet* GetUnitAnimSetDataAsset() const;
	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const;
	virtual FGameplayTag GetUnitTypeTag() const;
	
	virtual bool HasLevelSystem() const { return false; };
	virtual int32 GetUnitLevel() const { return 1; }
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual TSubclassOf<UUserWidget> GetStatusBarClass() const { return nullptr; }
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget);

	void ReAttachStatusBarToSocket() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> StatusBarComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	FName StatusBarSocketName = TEXT("HealthBar");
	
private:
	void InitAbilitySystem();
	void SetAnimSetData() const;
};
