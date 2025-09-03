// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectPC/Public/GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"

#include "DataAsset/Unit/PCDataAsset_UnitDefinitionReg.h"
#include "Character/UnitCharacter/PCCreepUnitCharacter.h"
#include "Character/UnitCharacter/PCAppearanceChangedHeroCharacter.h"
#include "Character/UnitCharacter/PCAppearanceFixedHeroCharacter.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Kismet/GameplayStatics.h"

void UPCUnitSpawnSubsystem::InitializeUnitSpawnConfig(const FSpawnSubsystemConfig& SpawnConfig)
{
	Registry = SpawnConfig.Registry;
	DefaultCreepClass = SpawnConfig.DefaultCreepClass;
	DefaultAppearanceChangedHeroClass = SpawnConfig.DefaultAppearanceChangedHeroClass;
	DefaultAppearanceFixedHeroClass = SpawnConfig.DefaultAppearanceFixedHeroClass;
	DefaultCreepStatusBarWidgetClass =SpawnConfig.CreepStatusBarWidgetClass;
	DefaultHeroStatusBarWidgetClass = SpawnConfig.HeroStatusBarWidgetClass;
	DefaultAIControllerClass = SpawnConfig.DefaultAIControllerClass;
}

void UPCUnitSpawnSubsystem::EnsureConfigFromGameState()
{
	if (Registry)
		return;
	
	if (const auto* GS = GetWorld()->GetGameState<APCCombatGameState>())
	{
		InitializeUnitSpawnConfig(GS->GetSpawnConfig());
	}
}

APCBaseUnitCharacter* UPCUnitSpawnSubsystem::SpawnUnitByTag(const FGameplayTag UnitTag, const FTransform& Transform,
                                                            const int32 UnitLevel, AActor* InOwner, APawn* InInstigator, ESpawnActorCollisionHandlingMethod HandlingMethod)
{
	// 유닛 스폰은 서버에서만, Listen Server 환경 고려 NM_Client로 판별
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
		return nullptr;

	const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(UnitTag);
	if (!Definition)
		return nullptr;

	TSubclassOf<APCBaseUnitCharacter> SpawnClass = ResolveSpawnUnitClass(Definition);

	APCBaseUnitCharacter* Unit = GetWorld()->SpawnActorDeferred<APCBaseUnitCharacter>(
		SpawnClass, Transform, InOwner, InInstigator, HandlingMethod);
	if (!Unit)
		return nullptr;

	Unit->SetUnitTag(UnitTag);
	
	ApplyDefinitionData(Unit, Definition);

	if (Unit->HasLevelSystem())
	{
		Unit->SetUnitLevel(UnitLevel);
	}

	UGameplayStatics::FinishSpawningActor(Unit, Transform);
	
	Unit->SetNetDormancy(DORM_Awake);
	Unit->ForceNetUpdate();
	
	return Unit;
}

const UPCDataAsset_UnitDefinition* UPCUnitSpawnSubsystem::ResolveDefinition(const FGameplayTag& UnitTag) const
{
	if (!Registry)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnUnit: Registry is null."));
		return nullptr;
	}
	
	UPCDataAsset_UnitDefinition* Def = Registry->FindUnitDefinition(UnitTag);
	if (!Def)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnUnit: Definition not found for tag: %s"), *UnitTag.ToString());
	}
	return Def;
}

TSubclassOf<APCBaseUnitCharacter> UPCUnitSpawnSubsystem::ResolveSpawnUnitClass(
	const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Definition) return nullptr;
	switch (Definition->ClassType)
	{
	case EUnitClassType::Creep:
		return DefaultCreepClass;
		
	case EUnitClassType::Hero_AppearanceChange:
		return DefaultAppearanceChangedHeroClass;
		
	case EUnitClassType::Hero_AppearanceFixed:
		return DefaultAppearanceFixedHeroClass;
		
	default:
		return nullptr;
	}
}

TSubclassOf<UUserWidget> UPCUnitSpawnSubsystem::ResolveStatusBarWidgetClass(
	const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Definition) return nullptr;
	switch (Definition->ClassType)
	{
	case EUnitClassType::Creep:
		return DefaultCreepStatusBarWidgetClass;
		
	case EUnitClassType::Hero_AppearanceChange:
	case EUnitClassType::Hero_AppearanceFixed:
		return DefaultHeroStatusBarWidgetClass;
		
	default:
		return nullptr;
	}
}

void UPCUnitSpawnSubsystem::ApplyDefinitionData(APCBaseUnitCharacter* Unit,
                                                const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Unit || !Definition)
		return;

	ApplyDefinitionDataVisuals(Unit, Definition);
	ApplyDefinitionDataServerOnly(Unit, Definition);
}

void UPCUnitSpawnSubsystem::ApplyDefinitionDataVisuals(APCBaseUnitCharacter* Unit,
	const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Unit || !Definition)
		return;

	if (USkeletalMeshComponent* SKComp = Unit->GetMesh())
	{
		if (Definition->Mesh)
			SKComp->SetSkeletalMesh(Definition->Mesh, true);
		
		if (Definition->AnimBP)
			SKComp->SetAnimInstanceClass(Definition->AnimBP);
		SKComp->SetVisibility(true, true);
	}
	
	if (Definition->UnitDataAsset)
		Unit->SetUnitDataAsset(Definition->UnitDataAsset);

	if (const TSubclassOf<UUserWidget> StatusBarWidgetClass = ResolveStatusBarWidgetClass(Definition))
	{
		Unit->SetStatusBarClass(StatusBarWidgetClass);
	}
}

void UPCUnitSpawnSubsystem::ApplyDefinitionDataServerOnly(APCBaseUnitCharacter* Unit,
	const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Unit || GetWorld()->GetNetMode() == NM_Client) return;

	// 공통 AI 클래스 할당 + AutoPossess
	if (DefaultAIControllerClass)
		Unit->AIControllerClass = DefaultAIControllerClass;

	Unit->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
