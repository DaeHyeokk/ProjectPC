// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCBaseUnitCharacter.generated.h"

class UPCUnitStatusBarWidget;
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
	virtual FGameplayTag GetUnitTypeTag() const;

	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const;
	FORCEINLINE virtual void SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset) { }

	FORCEINLINE void SetStatusBarClass(const TSubclassOf<UUserWidget>& InStatusBarClass) { StatusBarClass = InStatusBarClass; }
	
	FORCEINLINE virtual bool HasLevelSystem() const { return false; }
	FORCEINLINE virtual int32 GetUnitLevel() const { return 1; }
	virtual void SetUnitLevel(const int32 Level) { }

	// 유닛 태그 설정은 서버에서만
	FORCEINLINE void SetUnitTag(const FGameplayTag& InUnitTag) { if (HasAuthority()) UnitTag = InUnitTag; }
	FORCEINLINE const FGameplayTag& GetUnitTag() const { return UnitTag; }
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void InitStatusBarWidget(UUserWidget* StatusBarWidget);

	void ReAttachStatusBarToSocket() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> StatusBarComp;
	
	UPROPERTY(EditAnywhere, Category="Data")
	TSubclassOf<UUserWidget> StatusBarClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	FName StatusBarSocketName = TEXT("HealthBar");
	
private:
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_UnitTag, Category="Data")
	FGameplayTag UnitTag;

	UFUNCTION()
	void OnRep_UnitTag();
	
	void InitAbilitySystem();
	void SetAnimSetData() const;
};
