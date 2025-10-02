// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/GameStateBase.h"
#include "DataAsset/FrameWork/PCStageData.h"
#include "DataAsset/Projectile/PCDataAsset_ProjectilePoolData.h"
#include "GameFramework/PlayerState/PCLevelMaxXPData.h"
#include "PCCombatGameState.generated.h"

class UPCTileManager;
class APCCombatBoard;
class UPCShopManager;
class APCPlayerState;

USTRUCT(BlueprintType)
struct FSpawnSubsystemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Spawner|Registry")
	TObjectPtr<class UPCDataAsset_UnitDefinitionReg> Registry = nullptr;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCCreepUnitCharacter> DefaultCreepClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCAppearanceChangedHeroCharacter> DefaultAppearanceChangedHeroClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class APCAppearanceFixedHeroCharacter> DefaultAppearanceFixedHeroClass;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class UPCUnitStatusBarWidget> CreepStatusBarWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSubclassOf<class UPCHeroStatusBarWidget> HeroStatusBarWidgetClass;
	
	// == 공통 AI Controller (전 유닛 공유) ==
	UPROPERTY(EditAnywhere, Category="Spawner|AI")
	TSubclassOf<class APCUnitAIController> DefaultAIControllerClass;

	UPROPERTY(EditAnywhere, Category="Spawner|PreviewHero")
	TSoftClassPtr<class APCPreviewHeroActor> DefaultPreviewHeroClass;

	UPROPERTY(EditAnywhere, Category="Spawner|OutlineMaterial")
	TSoftObjectPtr<UMaterialInterface> DefaultOutlineMaterial;
};

USTRUCT(BlueprintType)
struct FStageRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 FloatIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 StageIdx = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RoundIdx = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 StepIdxInRound = 0;

	UPROPERTY(BlueprintReadOnly)
	EPCStageType Stage = EPCStageType::Setup;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ServerStartTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ServerEndTime = 0.f;
};

DECLARE_MULTICAST_DELEGATE(FOnStageRuntimeChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateTagChanged, const FGameplayTag);
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameState : public AGameStateBase, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	APCCombatGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

#pragma region GameLogic
	
	// 전체 게임 로직 관련 코드
public:

	// SeatIndex -> Board
	UPROPERTY(Replicated)
	TArray<APCCombatBoard*> SeatToBoard;

	UPROPERTY(Replicated)
	bool bBoardMappingComplete = false;

	UFUNCTION()
	void BuildSeatToBoardMap(const TArray<APCCombatBoard*>& Boards);

	UFUNCTION(BLueprintPure)
	APCCombatBoard* GetBoardBySeat(int32 PlayerSeatIndex) const;

	// GameStageState
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void SetStageRunTime(const FStageRuntimeState& NewState);

	UFUNCTION(BlueprintPure, Category = "Stage")
	const FStageRuntimeState& GetStageRunTime() const { return StageRuntimeState;}

	UPCTileManager* GetBattleTileManagerForSeat(int32 SeatIdx) const;
	APCCombatBoard* GetBattleBoardForSeat(int32 SeatIdx) const;

	


#pragma endregion GameLogic

#pragma region UI

public:

	// UI에서 바인딩할 값들
	UFUNCTION(BlueprintPure)
	float GetStageRemainingSeconds() const;

	UFUNCTION(BlueprintPure)
	float GetStageProgress() const;

	UFUNCTION(BlueprintPure)
	FString GetStageLabelString() const;

	UFUNCTION(BlueprintPure)
	EPCStageType GetCurrentStageType() const;

	FOnStageRuntimeChanged OnStageRuntimeChanged;
	FOnGameStateTagChanged OnGameStateTagChanged;

	UPROPERTY(ReplicatedUsing=OnRep_StageRunTime, BlueprintReadOnly, Category = "Stage")
	FStageRuntimeState StageRuntimeState;
	
protected:
	
	UFUNCTION()
	void OnRep_StageRunTime();

	
#pragma endregion UI
	
#pragma region Shop
	
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "ShopManager")
	UPCShopManager* ShopManager;

public:
	// GameState 생성자에서 생성하므로, ShopManager가 nullptr인 경우 바로 크래시
	// => 즉 GetShopManager()가 nullptr을 반환할 일은 없음
	FORCEINLINE UPCShopManager* GetShopManager() const { return ShopManager; }

#pragma endregion Shop

#pragma region Attribute
	
protected:
	// 플레이어 레벨 별 MaxXP 정보가 담긴 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* LevelMaxXPDataTable;

private:
	// 실제로 DataTable에서 가져온 정보를 저장할 배열
	TArray<FPCLevelMaxXPData> LevelMaxXPDataList;

public:
	int32 GetMaxXP(int32 PlayerLevel) const;

#pragma endregion Attribute

#pragma region Unit
	
private:
	UPROPERTY(EditAnywhere, Category="Unit Spawner Data")
	FSpawnSubsystemConfig SpawnConfig;

public:
	FORCEINLINE const FSpawnSubsystemConfig& GetSpawnConfig() const { return SpawnConfig; }
	
#pragma endregion Unit

public:
	void SetGameStateTag(const FGameplayTag& InGameStateTag);
	UFUNCTION(BlueprintPure)
	const FGameplayTag& GetGameStateTag() const { return GameStateTag; }
	bool IsCombatActive() const { return GameStateTag.MatchesTag(GameStateTags::Game_State_Combat); }

	bool bIsbattle() const { return GameStateTag == GameStateTags::Game_State_Combat_Preparation || GameStateTag == GameStateTags::Game_State_Combat_Active; }
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_GameStateTag)
	FGameplayTag GameStateTag;

	UFUNCTION()
	void OnRep_GameStateTag();
	
	// ==== 전투 시스템 | BT 관련 ====
protected:
	//BT Decorator에서 Game State Tag 정보 참조하기 위해
	//IGameplayTagAssetInterface 상속 받아서 오버라이드한 함수
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	
	// TEST CODE //
public:
	UFUNCTION(BlueprintCallable)
	void Test_StartCombat() { SetGameStateTag(GameStateTags::Game_State_Combat_Active); }

#pragma region ObjectPool

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ObjectPool")
	UPCDataAsset_ProjectilePoolData* ProjectilePoolData;
	
#pragma endregion ObjectPool
	
#pragma region TemplateFunc
	
private:
	// DataTable을 읽어 아웃파라미터로 TArray에 값을 넘기는 템플릿 함수
	template<typename T>
	void LoadDataTable(UDataTable* DataTable, TArray<T>& OutDataList, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutDataList.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		// DataTable의 Row수만큼 메모리 미리 확보
		OutDataList.Reserve(RowPtrs.Num());
		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				OutDataList.Add(*Row);
			}
		}
	}

#pragma endregion TemplateFunc
};
