// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataAsset/Unit/PCDataAsset_UnitDefinition.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCUnitSpawnSubsystem.generated.h"

/**
 * 
 */

class APCPlayerState;
class APCCombatBoard;
class APCHeroUnitCharacter;
class APCPreviewHeroActor;
class UPCHeroStatusBarWidget;
class UPCUnitStatusBarWidget;
struct FSpawnSubsystemConfig;
class UBehaviorTree;
class APCUnitAIController;
class APCAppearanceFixedHeroCharacter;
class APCAppearanceChangedHeroCharacter;
class APCCreepUnitCharacter;
class UPCDataAsset_UnitDefinitionReg;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnitSpawnedNative, APCBaseUnitCharacter*, int32 );

UCLASS()
class PROJECTPC_API UPCUnitSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UPCDataAsset_UnitDefinitionReg> Registry = nullptr;

	UPROPERTY()
	TSubclassOf<APCCreepUnitCharacter> DefaultCreepClass;
	
	UPROPERTY()
	TSubclassOf<APCAppearanceChangedHeroCharacter> DefaultAppearanceChangedHeroClass;
	
	UPROPERTY()
	TSubclassOf<APCAppearanceFixedHeroCharacter> DefaultAppearanceFixedHeroClass;

	UPROPERTY()
	TSubclassOf<UPCUnitStatusBarWidget> DefaultCreepStatusBarWidgetClass;

	UPROPERTY()
	TSubclassOf<UPCHeroStatusBarWidget> DefaultHeroStatusBarWidgetClass;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> DefaultOutlineMaterial;
	
	// == 공통 AI / BT (전 유닛 공유) ==
	UPROPERTY(EditAnywhere, Category="Spawner|AI")
	TSubclassOf<APCUnitAIController> DefaultAIControllerClass;

public:
	void InitializeUnitSpawnConfig(const FSpawnSubsystemConfig& SpawnConfig);
	void EnsureConfigFromGameState();
	
	UFUNCTION(BlueprintCallable, Category="Spawner")
	APCBaseUnitCharacter* SpawnUnitByTag(
		const FGameplayTag UnitTag,
		const int32 TeamIndex = 255,
		const int32 UnitLevel = 1,
		AActor* InOwner = nullptr,
		APawn* InInstigator = nullptr,
		ESpawnActorCollisionHandlingMethod HandlingMethod =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	const UPCDataAsset_UnitDefinition* ResolveDefinition(const FGameplayTag& UnitTag) const;
	void ApplyDefinitionDataVisuals(APCBaseUnitCharacter* Unit, const UPCDataAsset_UnitDefinition* Definition) const;
	
private:
	TSubclassOf<APCBaseUnitCharacter> ResolveSpawnUnitClass(const UPCDataAsset_UnitDefinition* Definition) const;
	TSubclassOf<UUserWidget> ResolveStatusBarWidgetClass(const UPCDataAsset_UnitDefinition* Definition) const;
	void ApplyDefinitionData(APCBaseUnitCharacter* Unit, const UPCDataAsset_UnitDefinition* Definition) const;
	void ApplyDefinitionDataServerOnly(APCBaseUnitCharacter* Unit, const UPCDataAsset_UnitDefinition* Definition) const;

private:
	UPROPERTY()
	TSubclassOf<APCPreviewHeroActor> DefaultPreviewHeroClass;

public:
	UFUNCTION(BlueprintCallable, Category="Spawner")
	APCPreviewHeroActor* SpawnPreviewHeroBySourceHero(
		APCHeroUnitCharacter* SourceHero,
		AActor* InOwner = nullptr,
		APawn* InInstigator = nullptr,
		ESpawnActorCollisionHandlingMethod HandlingMethod =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn) const;


	// 전투중 스폰 델리게이트
public:
	FOnUnitSpawnedNative OnUnitSpawned;
	
};
