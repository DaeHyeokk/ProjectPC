// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GenericTeamAgentInterface.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "PCBaseUnitCharacter.generated.h"

class UPCDataAsset_UnitAbilityConfig;
class UPCUnitStatusBarWidget;
class UWidgetComponent;
class UGameplayAbility;
class UPCUnitAttributeSet;
class UPCUnitAbilitySystemComponent;

UCLASS()
class PROJECTPC_API APCBaseUnitCharacter : public ACharacter, public IAbilitySystemInterface,
										public IGenericTeamAgentInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	APCBaseUnitCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const;
	UPCDataAsset_UnitAnimSet* GetUnitAnimSetDataAsset() const;
	const UPCDataAsset_UnitAbilityConfig* GetUnitAbilityConfigDataAsset() const;
	virtual FGameplayTag GetUnitTypeTag() const;
	
	virtual const UPCDataAsset_BaseUnitData* GetUnitDataAsset() const;
	virtual void SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset) { }

	void SetStatusBarClass(const TSubclassOf<UUserWidget>& InStatusBarClass) { StatusBarClass = InStatusBarClass; }
	
	virtual bool HasLevelSystem() const { return false; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	virtual int32 GetUnitLevel() const { return 1; }
	virtual void SetUnitLevel(const int32 Level) { }

	// Unit Tag 설정은 서버에서만 실행
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	void SetUnitTag(const FGameplayTag& InUnitTag) { if (HasAuthority()) UnitTag = InUnitTag; }
	UFUNCTION(BlueprintCallable, Category="Unit Data")
	FORCEINLINE FGameplayTag GetUnitTag() const { return UnitTag; }

	FORCEINLINE const UWidgetComponent* GetStatusBarComponent() const { return StatusBarComp; }
	FORCEINLINE FName GetStatusBarSocketName() const { return StatusBarSocketName; }
	
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
	
	UFUNCTION()
	void OnRep_UnitTag();

	void PushTeamIndexToController() const;
	
	void InitAbilitySystem();
	void SetAnimSetData() const;

	// 전투 시스템 관련 //
public:
	// Team Index, 위치한 CombatBoard 설정은 서버에서만 실행
	UFUNCTION(BlueprintCallable, Category="Combat")
	void SetTeamIndex(const int32 InTeamID) { if (HasAuthority()) TeamIndex = InTeamID; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	int32 GetTeamIndex() const { return TeamIndex; }

	virtual FGenericTeamId GetGenericTeamId() const override final;
	
	UFUNCTION(BlueprintCallable, Category="Combat")
	void SetOnCombatBoard(APCCombatBoard* InCombatBoard) { if (HasAuthority()) OnCombatBoard = InCombatBoard; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	FORCEINLINE APCCombatBoard* GetOnCombatBoard() const { return OnCombatBoard.Get(); }

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ChangedOnTile(const bool IsOnField);

	UFUNCTION(BlueprintCallable, Category="Combat")
	bool IsOnField() const { return bIsOnField; }
	
	UFUNCTION(BlueprintCallable, Category="Combat")
	bool IsCombatActive() const { return bIsCombatActive; }

protected:
	UPROPERTY()
	TObjectPtr<APCCombatBoard> OnCombatBoard;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_IsOnField, Category="Combat")
	bool bIsOnField = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_IsCombatActive, Category="Combat")
	bool bIsCombatActive = false;
	
	UFUNCTION()
	void OnRep_IsOnField();

	void BindCombatState();	// 필드에 올라갈 때 바인딩
	void UnbindCombatState(); // 벤치로 가거나 사망하면 바인딩 해제
	void HandleGameStateChanged(const FGameplayTag& GameStateTag);

	UFUNCTION()
	void OnRep_IsCombatActive() const;
	
	FDelegateHandle GameStateChangedHandle;

	// ==== 전투 시스템 | BT 관련 ====
protected:
	// BT Decorator에서 ASC에 부여된 GameplayTag 정보 참조하기 위해
	// IGameplayTagAssetInterface 상속 받아서 오버라이드한 함수
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	
};
