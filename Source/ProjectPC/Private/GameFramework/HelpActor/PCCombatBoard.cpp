// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCombatBoard.h"

#include "Camera/CameraComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"


APCCombatBoard::APCCombatBoard()
{ 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	FieldRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FieldRoot"));
	FieldRoot->SetupAttachment(SceneRoot);

	BenchRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BenchRoot"));
	BenchRoot->SetupAttachment(SceneRoot);

	// HISM
	FieldHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FieldHISM"));
	FieldHISM->SetupAttachment(SceneRoot);
	FieldHISM->SetMobility(EComponentMobility::Static);
	FieldHISM->NumCustomDataFloats = 4;

	BenchHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BenchHISM"));
	BenchHISM->SetupAttachment(SceneRoot);
	BenchHISM->SetMobility(EComponentMobility::Static);
	BenchHISM->NumCustomDataFloats = 4;
	
	// 카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->bDoCollisionTest = false;
	SpringArm->TargetArmLength = 2200.f;
	SpringArm->SetRelativeLocation(FVector(0,0,1200));
	SpringArm->SetRelativeRotation(FRotator(-55,0,0));

	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SpringArm);
	BoardCamera->FieldOfView = 55.f;

	// TIle Manger
	TileManager = CreateDefaultSubobject<UPCTileManager>(TEXT("TileManager"));
}


void APCCombatBoard::BeginPlay()
{
	Super::BeginPlay();
	RebuildAnchors();
	RebuildTilesFromMarkers();

	if (TileManager)
	{
		TileManager->QuickSetUp();
		TileManager->DebugDrawTiles(true);
	}
	
}

#if WITH_EDITOR
void APCCombatBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildAnchors();
	RebuildTilesFromMarkers();
	
	
		
}
#endif

USceneComponent* APCCombatBoard::Resolve(const FComponentReference& Ref) const
{
	return Cast<USceneComponent>(Ref.GetComponent(const_cast<APCCombatBoard*>(this)));
}

void APCCombatBoard::RebuildTilesFromMarkers()
{
	CollectTileMarkers();
	BuildHISM();
}

static int32 NameSuffixToIndex(const FString& Name, const FString& Prefix)
{
	if (!Name.StartsWith(Prefix))
		return -1;
	int32 Index = -1;
	LexTryParseString(Index, *Name.RightChop(Prefix.Len()));
	return Index;
}

void APCCombatBoard::CollectTileMarkers()
{
	FieldTiles.Reset();
	BenchTiles.Reset();

	auto Gather = [&](USceneComponent* Root, const FString& Prefix, TArray<FTileInfo>& Out)
	{
		if (!Root) return;
		TArray<USceneComponent*> Children;
		Root->GetChildrenComponents(false, Children);
		Children.Sort([&](const USceneComponent& ARoot, const USceneComponent& BRoot)
		{
			const int ARootIndex = NameSuffixToIndex(ARoot.GetName(), Prefix);
			const int BRootIndex = NameSuffixToIndex(BRoot.GetName(), Prefix);
			return ARootIndex < BRootIndex;
		});
		for (USceneComponent* SceneComponent : Children)
		{
			FTileInfo TileInfo;
			TileInfo.WorldTransform = SceneComponent->GetComponentTransform();
			Out.Add(TileInfo);
		}
	};

	Gather(FieldRoot, FieldPrefix.ToString(), FieldTiles);
	Gather(BenchRoot, BenchPrefix.ToString(), BenchTiles);
}


void APCCombatBoard::BuildHISM()
{
	FieldHISM->ClearInstances();
	BenchHISM->ClearInstances();

	if (HexTileMesh)
		FieldHISM->SetStaticMesh(HexTileMesh);
	if (HexTileMaterial)
		FieldHISM->SetMaterial(0, HexTileMaterial);
	if (HexTileOverlayMaterial)
		FieldHISM->SetOverlayMaterial(HexTileOverlayMaterial);
	if (BenchTileMesh)
		BenchHISM->SetStaticMesh(BenchTileMesh);
	if (BenchTileMaterial)
		BenchHISM->SetMaterial(0, BenchTileMaterial);
	if (BenchTileOverlayMaterial)
		BenchHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	
	Field_InstanceToXY.Reset();
	Bench_InstanceToXY.Reset();

	// 필드 인스턴스
	for (int32 i = 0; i < FieldTiles.Num(); ++i)
	{
		const int32 InstanceIndex = FieldHISM->AddInstance(FieldTiles[i].WorldTransform, true);
		if (InstanceIndex != INDEX_NONE)
		{
			Field_InstanceToXY.Add(FIntPoint(i,0));
		}
	}

	for (int32 i = 0; i < BenchTiles.Num(); ++i)
	{
		const int32 InstanceIndex = BenchHISM->AddInstance(BenchTiles[i].WorldTransform, true);
		if (InstanceIndex != INDEX_NONE)
		{
			Bench_InstanceToXY.Add(FIntPoint(i,0));
		}
	}

	FieldHISM->BuildTreeIfOutdated(true,true);
	BenchHISM->BuildTreeIfOutdated(true,true);
}

void APCCombatBoard::RebuildAnchors()
{
	auto Make = [&](USceneComponent*& Out, const FComponentReference& ParentRef, FName Socket)
	{
		USceneComponent* Parent = Resolve(ParentRef);
		if (!Parent || Socket.IsNone())
		{
			Out = nullptr;
			return;
		}

		if (!Out)
		{
			Out = NewObject<USceneComponent>(this);
			Out->RegisterComponent();
		}
		Out->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetIncludingScale, Socket);
		Out->SetRelativeLocation(SeatExtraOffset);
		Out->SetRelativeRotation(FRotator::ZeroRotator);
	};
	Make(PlayerSeatAnchor, PlayerSeatParent, PlayerSeatSocket);
	Make(EnemySeatAnchor, EnemySeatParent, EnemySeatSocket);
}


FTransform APCCombatBoard::GetPlayerSeatTransform() const
{
	return PlayerSeatAnchor ? PlayerSeatAnchor->GetComponentTransform() : GetActorTransform();
}

FTransform APCCombatBoard::GetEnemySeatTransform() const
{
	return EnemySeatAnchor ? EnemySeatAnchor->GetComponentTransform() : GetActorTransform();
}

void APCCombatBoard::ApplyLocalBottomView(class APlayerController* PlayerController, int32 MySeatIndex, float Blend)
{
	if (!PlayerController)
		return;
	PlayerController->SetViewTargetWithBlend(this, Blend);
}

void APCCombatBoard::ApplyBattleCamera(class APCCombatPlayerController* PCPlayerController, bool bFlipYaw180,
	float Blend)
{
	if (!PCPlayerController || !SpringArm)
		return;
	
	if (bFlipYaw180)
	{
		SpringArm->SetRelativeLocation(BattleCameraChangeLocation);
		SpringArm->SetRelativeRotation(BattleCameraChangeRotation);
	}
	PCPlayerController->SetViewTargetWithBlend(this, Blend);
}

APCHeroUnitCharacter* APCCombatBoard::GetUnitAt(int32 Row, int32 Col) const
{
	return TileManager ? TileManager->GetFieldUnit(Row, Col) : nullptr;
}

FVector APCCombatBoard::GetTileWorldLocation(int32 Row, int32 Col) const
{
	return TileManager ? TileManager->GetTileWorldPosition(Row, Col) : FVector::ZeroVector;
}

APCHeroUnitCharacter* APCCombatBoard::GetBenchUnitAt(int32 BenchIndex) const
{
	return TileManager ? TileManager->GetBenchUnit(BenchIndex) : nullptr;
}

FVector APCCombatBoard::GetBenchWorldLocation(int32 BenchIndex) const
{
	return TileManager ? TileManager->GetBenchWorldPosition(BenchIndex) : FVector::ZeroVector;
}



