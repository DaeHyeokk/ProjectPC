// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCBaseUnitCharacter.generated.h"

class APCCombatBoard;
class UPCUnitStatusBarWidget;
class UWidgetComponent;
class UGameplayAbility;
class UPCUnitAttributeSet;
class UPCUnitAbilitySystemComponent;

UCLASS()
class PROJECTPC_API APCBaseUnitCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
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
	virtual void SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset) { }

	void SetStatusBarClass(const TSubclassOf<UUserWidget>& InStatusBarClass) { StatusBarClass = InStatusBarClass; }
	
	virtual bool HasLevelSystem() const { return false; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	virtual int32 GetUnitLevel() const { return 1; }
	virtual void SetUnitLevel(const int32 Level) { }

	// Unit Tag, Team Index, 위치한 CombatBoard 설정은 서버에서만 실행
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	void SetUnitTag(const FGameplayTag& InUnitTag) { if (HasAuthority()) UnitTag = InUnitTag; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	FORCEINLINE FGameplayTag GetUnitTag() const { return UnitTag; }

	UFUNCTION(BlueprintCallable, Category="Unit Data")
	void SetTeamIndex(const int32 InTeamID) { if (HasAuthority()) TeamIndex = InTeamID; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	int32 GetTeamIndex() const { return TeamIndex; }

	virtual FGenericTeamId GetGenericTeamId() const override final;
	
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	void SetOnCombatBoard(APCCombatBoard* InCombatBoardIndex) { if (HasAuthority()) OnCombatBoard = InCombatBoardIndex; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	FORCEINLINE const APCCombatBoard* GetOnCombatBoard() const { return OnCombatBoard.Get(); }
	
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
	
protected:
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_UnitTag, Category="Data")
	FGameplayTag UnitTag;

	UPROPERTY(EditDefaultsOnly, Replicated, Category="Data")
	int32 TeamIndex = -1;
	
	TWeakObjectPtr<APCCombatBoard> OnCombatBoard = nullptr;
	
	UFUNCTION()
	void OnRep_UnitTag();

	void PushTeamIndexToController() const;
	
	void InitAbilitySystem();
	void SetAnimSetData() const;
};
