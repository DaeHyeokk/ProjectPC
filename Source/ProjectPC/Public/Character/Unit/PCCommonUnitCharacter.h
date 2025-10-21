// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "GameFramework/Character.h"
#include "PCCommonUnitCharacter.generated.h"

UCLASS(Abstract)
class PROJECTPC_API APCCommonUnitCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return nullptr; }
	virtual TArray<FGameplayTag> GetEquipItemTags() const PURE_VIRTUAL(APCCommonUnitCharacter::GetEquipItemList, return TArray<FGameplayTag>(););


	void SetUnitTag(const FGameplayTag& InUnitTag) { if (HasAuthority()) { UnitTag = InUnitTag; }}
	const FGameplayTag& GetUnitTag() const { return UnitTag; }

	void SetUnitRecommendedPosition(const EUnitRecommendedPosition InRecommendedPosition) { RecommendedPosition = InRecommendedPosition; }
	const EUnitRecommendedPosition& GetUnitRecommendedPosition() const { return RecommendedPosition; }

	UFUNCTION(BlueprintCallable, Category="Unit Data")
	virtual int32 GetUnitLevel() const { return 1; }
	virtual bool HasLevelSystem() const { return false; }
	virtual void SetUnitLevel(const int32 Level) { }
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_UnitTag, Category="Data")
	FGameplayTag UnitTag;

	UFUNCTION()
	virtual void OnRep_UnitTag() PURE_VIRTUAL(APCCommonUnitCharacter::OnRep_UnitTag);
	
	EUnitRecommendedPosition RecommendedPosition = EUnitRecommendedPosition::FrontLine;
};
