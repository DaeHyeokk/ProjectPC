// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectPC/Public/GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"

#include "Animation/Unit/PCPreviewHeroAnimInstance.h"
#include "DataAsset/Unit/PCDataAsset_UnitDefinitionReg.h"
#include "Character/Unit/PCCreepUnitCharacter.h"
#include "Character/Unit/PCAppearanceChangedHeroCharacter.h"
#include "Character/Unit/PCAppearanceFixedHeroCharacter.h"
#include "Character/Unit/PCCarouselHeroCharacter.h"
#include "Character/Unit/PCPreloadHeroActor.h"
#include "Character/Unit/PCPreviewHeroActor.h"
#include "Component/PCUnitEquipmentComponent.h"
#include "UI/Unit/PCHeroStatusBarWidget.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Kismet/GameplayStatics.h"

void UPCUnitSpawnSubsystem::InitializeUnitSpawnConfig(const FSpawnSubsystemConfig& SpawnConfig)
{
	Registry = SpawnConfig.Registry.LoadSynchronous();
	DefaultCreepClass = SpawnConfig.DefaultCreepClass.LoadSynchronous();
	DefaultAppearanceChangedHeroClass = SpawnConfig.DefaultAppearanceChangedHeroClass.LoadSynchronous();
	DefaultAppearanceFixedHeroClass = SpawnConfig.DefaultAppearanceFixedHeroClass.LoadSynchronous();
	DefaultCreepStatusBarWidgetClass =SpawnConfig.CreepStatusBarWidgetClass.LoadSynchronous();
	DefaultHeroStatusBarWidgetClass = SpawnConfig.HeroStatusBarWidgetClass.LoadSynchronous();
	DefaultAIControllerClass = SpawnConfig.DefaultAIControllerClass.LoadSynchronous();
	DefaultPreviewHeroClass = SpawnConfig.DefaultPreviewHeroClass.LoadSynchronous();
	DefaultCarouselHeroClass = SpawnConfig.DefaultCarouselHeroClass.LoadSynchronous();
	DefaultPreloadActorClass = SpawnConfig.DefaultPreloadActorClass.LoadSynchronous();
	DefaultOutlineMaterial = SpawnConfig.DefaultOutlineMaterial.LoadSynchronous();
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

void UPCUnitSpawnSubsystem::PreloadAllHeroUnit(const FVector& SpawnLocation) const
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client || !Registry)
		return;
	
	FGameplayTagContainer AllHeroTags;
	Registry->GatherAllRegisteredHeroTags(AllHeroTags);
	
	for (const FGameplayTag& HeroTag : AllHeroTags)
	{
		for (int32 Level = 1; Level <=3; ++Level)
		{
			if (APCPreloadHeroActor* PreloadActor = SpawnPreloadActorByTag(HeroTag, Level))
			{
				PreloadActor->SetActorLocation(SpawnLocation);
				PreloadActor->SetActorHiddenInGame(true);
				PreloadActor->SetActorEnableCollision(false);
				PreloadActor->SetLifeSpan(3.f);
			}
		}
	}
}

APCPreloadHeroActor* UPCUnitSpawnSubsystem::SpawnPreloadActorByTag(const FGameplayTag UnitTag, const int32 UnitLevel,
	ESpawnActorCollisionHandlingMethod HandlingMethod) const
{
	// 스폰은 서버에서만, Listen Server 환경 고려 NM_Client로 판별
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
		return nullptr;

	const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(UnitTag);
	if (!Definition)
		return nullptr;

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(FVector({0.f,0.f,9999.f}));
	
	APCPreloadHeroActor* PreloadActor = GetWorld()->SpawnActorDeferred<APCPreloadHeroActor>(
		DefaultPreloadActorClass,
		SpawnTransform,
		nullptr,
		nullptr,
		HandlingMethod);
	
	if (!PreloadActor)
		return nullptr;

	PreloadActor->SetUnitLevel(UnitLevel);
	PreloadActor->SetUnitTag(UnitTag);
	ApplyDefinitionData(PreloadActor, UnitLevel, Definition);
	
	UGameplayStatics::FinishSpawningActor(PreloadActor, SpawnTransform);
	
	PreloadActor->SetNetDormancy(DORM_Awake);
	PreloadActor->ForceNetUpdate();
	
	return PreloadActor;
}

APCBaseUnitCharacter* UPCUnitSpawnSubsystem::SpawnUnitByTag(const FGameplayTag UnitTag, const int32 TeamIndex,
                                                            const int32 UnitLevel, APCPlayerState* InOwnerPS, AActor* InOwner, APawn* InInstigator, ESpawnActorCollisionHandlingMethod HandlingMethod)
{
	// 유닛 스폰은 서버에서만, Listen Server 환경 고려 NM_Client로 판별
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
		return nullptr;

	const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(UnitTag);
	if (!Definition)
		return nullptr;

	TSubclassOf<APCBaseUnitCharacter> SpawnClass = ResolveSpawnUnitClass(Definition);

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(FVector({0.f,0.f,9999.f}));
	
	APCBaseUnitCharacter* Unit = GetWorld()->SpawnActorDeferred<APCBaseUnitCharacter>(
		SpawnClass,
		SpawnTransform,
		InOwner,
		InInstigator,
		HandlingMethod);
	
	if (!Unit)
		return nullptr;
	
	Unit->SetOwnerPlayerState(InOwnerPS);
	Unit->SetTeamIndex(TeamIndex);
	Unit->SetUnitTag(UnitTag);
	ApplyDefinitionData(Unit, Definition);
	
	if (Unit->HasLevelSystem())
	{
		Unit->SetUnitLevel(UnitLevel);
	}

	UGameplayStatics::FinishSpawningActor(Unit, SpawnTransform);
	
	Unit->SetNetDormancy(DORM_Awake);
	Unit->ForceNetUpdate();

	OnUnitSpawned.Broadcast(Unit, TeamIndex);
	
	return Unit;
}

APCBaseUnitCharacter* UPCUnitSpawnSubsystem::SpawnCloneUnitBySourceUnit(const APCBaseUnitCharacter* SourceUnit)
{
	if (!SourceUnit)
		return nullptr;
	
	const FGameplayTag& UnitTag = SourceUnit->GetUnitTag();
	const int32 TeamIndex = SourceUnit->GetTeamIndex();
	const int32 UnitLevel = SourceUnit->GetUnitLevel();
	APCPlayerState* OwnerPS = SourceUnit->GetOwnerPlayerState();
	
	APCBaseUnitCharacter* CloneUnit = SpawnUnitByTag(UnitTag, TeamIndex, UnitLevel, OwnerPS);
	if (!CloneUnit)
		return nullptr;
	
	UPCUnitEquipmentComponent* SourceEquipmentComp = SourceUnit->GetEquipmentComponent();
	UPCUnitEquipmentComponent* CloneEquipmentComp = CloneUnit->GetEquipmentComponent();
	if (SourceEquipmentComp && CloneEquipmentComp)
	{
		const TArray<FGameplayTag>& EquipItemTags = SourceEquipmentComp->GetSlotItemTags();

		for (const FGameplayTag& ItemTag : EquipItemTags)
		{
			if (ItemTag.IsValid())
			{
				CloneEquipmentComp->TryEquipItem(ItemTag);
			}
		}
	}

	return CloneUnit;
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
		Unit->SetOutlineMID(DefaultOutlineMaterial);
		
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

USoundCue* UPCUnitSpawnSubsystem::GetLevelStartSoundCueByUnitTag(const FGameplayTag& UnitTag) const
{
	if (const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(UnitTag))
	{
		return Definition->LevelStartSoundCue;
	}

	return nullptr;
}

void UPCUnitSpawnSubsystem::ApplyDefinitionData(APCCarouselHeroCharacter* CarouselHero,
                                                const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!CarouselHero || !Definition)
		return;

	if (USkeletalMeshComponent* SKComp = CarouselHero->GetMesh())
	{
		if (Definition->Mesh)
			SKComp->SetSkeletalMesh(Definition->Mesh, true);
		
		if (Definition->CarouselHeroAnimBP)
			SKComp->SetAnimInstanceClass(Definition->CarouselHeroAnimBP);
		
		SKComp->SetVisibility(true, true);
	}

	if (UPCDataAsset_HeroUnitData* HeroData = Cast<UPCDataAsset_HeroUnitData>(Definition->UnitDataAsset))
	{
		CarouselHero->SetHeroUnitDataAsset(HeroData);
	}
}

void UPCUnitSpawnSubsystem::ApplyDefinitionData(const APCPreloadHeroActor* PreloadActor, int32 UnitLevel,
	const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!PreloadActor || !Definition)
		return;

	if (USkeletalMeshComponent* SKComp = PreloadActor->GetMesh())
	{
		switch (UnitLevel)
		{
		case 1:
			if (Definition->Mesh)
				SKComp->SetSkeletalMesh(Definition->Mesh, true);
			break;

		case 2:
			if (Definition->Mesh_Level2)
				SKComp->SetSkeletalMesh(Definition->Mesh_Level2, true);
			break;

		case 3:
			if (Definition->Mesh_Level3)
				SKComp->SetSkeletalMesh(Definition->Mesh_Level3, true);
			break;
			
		default:
			break;
		}
		
		SKComp->SetVisibility(true, true);
	}
}

void UPCUnitSpawnSubsystem::ApplyDefinitionDataForCarouselServerOnly(APCCarouselHeroCharacter* Carousel,
                                                                     const UPCDataAsset_UnitDefinition* Definition) const
{
	if (!Carousel || !Definition || GetWorld()->GetNetMode() == NM_Client)
		return;

	ApplyDefinitionData(Carousel, Definition);
	
	// 움직임/회전이 있으므로 도르만시 깨어있게 유지
	Carousel->SetNetDormancy(DORM_Awake);
	Carousel->ForceNetUpdate();
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

APCPreviewHeroActor* UPCUnitSpawnSubsystem::SpawnPreviewHeroBySourceHero(APCHeroUnitCharacter* SourceHero,
	AActor* InOwner, APawn* InInstigator,
	ESpawnActorCollisionHandlingMethod HandlingMethod) const
{
	// 프리뷰 유닛 스폰은 클라에서만, Listen Server 환경 고려 NM_DedicatedServer로 판별
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_DedicatedServer)
		return nullptr;

	const TSubclassOf<APCPreviewHeroActor> SpawnClass = DefaultPreviewHeroClass;
	if (!SpawnClass)
		return nullptr;
	if (!SourceHero)
		return nullptr;
	
	const USkeletalMeshComponent* SourceMesh = SourceHero->GetMesh();
	const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(SourceHero->GetUnitTag());
	if (!SourceMesh || !Definition)
		return nullptr;

	const TSubclassOf<UUserWidget>& SourceStatusBarWidgetClass = ResolveStatusBarWidgetClass(Definition);
	const TSubclassOf<UAnimInstance>& PreviewHeroAnimBP = Definition->PreviewHeroAnimBP;
	
	if (!SourceStatusBarWidgetClass || !PreviewHeroAnimBP)
		return nullptr;

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(FVector({0.f,0.f,9999.f}));
	
	APCPreviewHeroActor* PreviewHero = GetWorld()->SpawnActorDeferred<APCPreviewHeroActor>(
		SpawnClass,
		SpawnTransform,
		InOwner,
		InInstigator,
		HandlingMethod);

	if (!PreviewHero)
		return nullptr;

	PreviewHero->InitializeFromSourceHero(SourceHero, PreviewHeroAnimBP, SourceStatusBarWidgetClass);
	
	UGameplayStatics::FinishSpawningActor(PreviewHero, SpawnTransform);
	
	PreviewHero->SetActorHiddenInGame(false);

	return PreviewHero;
}

APCCarouselHeroCharacter* UPCUnitSpawnSubsystem::SpawnCarouselHeroByTag(const FGameplayTag UnitTag, const FGameplayTag ItemTag,
	AActor* InOwner, APawn* InInstigator, ESpawnActorCollisionHandlingMethod HandlingMethod)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
		return nullptr;
	
	const UPCDataAsset_UnitDefinition* Definition = ResolveDefinition(UnitTag);
	if (!Definition)
		return nullptr;

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(FVector(0.f, 0.f, 99999.f));

	APCCarouselHeroCharacter* Carousel = GetWorld()->SpawnActorDeferred<APCCarouselHeroCharacter>(
		DefaultCarouselHeroClass, SpawnTransform, InOwner, InInstigator, HandlingMethod);

	if (!Carousel)
		return nullptr;

	Carousel->SetUnitTag(UnitTag);
	Carousel->SetItemTag(ItemTag);
	
	ApplyDefinitionDataForCarouselServerOnly(Carousel, Definition);

	UGameplayStatics::FinishSpawningActor(Carousel, SpawnTransform);

	return Carousel;
}