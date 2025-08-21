// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCBaseUnitCharacter.generated.h"

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
	const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const;
	virtual FGameplayTag GetUnitTypeTag() const;
	
	virtual bool HasLevelSystem() const { return false; };
	virtual int32 GetUnitLevel() const { return 1; }
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditAnywhere, Category="Data")
	TObjectPtr<UPCDataAsset_BaseUnitData> BaseUnitDataAsset;
	
	void InitAbilitySystem();
};
