// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCombatBoard.h"

#include "Camera/CameraComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Net/UnrealNetwork.h"


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

	EnemyBenchRoot = CreateDefaultSubobject<USceneComponent>(TEXT("EnemyRoot"));
	EnemyBenchRoot->SetupAttachment(SceneRoot);

	// HISM
	FieldHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FieldHISM"));
	FieldHISM->SetupAttachment(SceneRoot);
	FieldHISM->SetMobility(EComponentMobility::Static);
	FieldHISM->NumCustomDataFloats = 4;

	BenchHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BenchHISM"));
	BenchHISM->SetupAttachment(SceneRoot);
	BenchHISM->SetMobility(EComponentMobility::Static);
	BenchHISM->NumCustomDataFloats = 4;

	EnemyHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("EnemyHISM"));
	EnemyHISM->SetupAttachment(SceneRoot);
	EnemyHISM->SetMobility(EComponentMobility::Static);
	EnemyHISM->NumCustomDataFloats = 4;
	
	// 카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->bDoCollisionTest = false;
	SpringArm->TargetArmLength = 3000.f;
	SpringArm->SetRelativeLocation(HomeCam_LocPreset);
	SpringArm->SetRelativeRotation(HomeCam_RocPreset);

	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SpringArm);
	BoardCamera->FieldOfView = 55.f;

	// TIle Manger
	TileManager = CreateDefaultSubobject<UPCTileManager>(TEXT("TileManager"));
}

void APCCombatBoard::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCCombatBoard, BoardSeatIndex);
}


void APCCombatBoard::BeginPlay()
{
	Super::BeginPlay();
	RebuildAnchors();
	RebuildTilesFromMarkers();

	if (TileManager)
	{
		TileManager->QuickSetUp();
		TileManager->DebugLogField(true,true,TEXT("TileManger"));
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

int32 APCCombatBoard::GetFirstEmptyBenchIndex(int32 SeatIndex) const
{
	// for (int32 Index = 0; Index < TileManager->BenchSize; ++Index)
		// 헤더에는 TileManager의 BenchSize가 9지만, 런타임에 18이 되므로 임시로 하드 코딩

	if (!TileManager) return INDEX_NONE;

	const bool bEnemySide = (SeatIndex != BoardSeatIndex); // 게스트면 enemy side
	const int32 N = TileManager->BenchSlotsPerSide;
	for (int32 local = 0; local < N; ++local)
	{
		const int32 gi = TileManager->MakeGlobalBenchIndex(bEnemySide, local);
		if (TileManager->GetBenchUnit(gi) == nullptr)
			return gi;
	}
	return INDEX_NONE;
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
	EnemyTiles.Reset();

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
	Gather(EnemyBenchRoot, BenchPrefix.ToString(), EnemyTiles);
}


void APCCombatBoard::BuildHISM()
{
	FieldHISM->ClearInstances();
	BenchHISM->ClearInstances();
	EnemyHISM->ClearInstances();

	if (HexTileMesh)
		FieldHISM->SetStaticMesh(HexTileMesh);
	if (HexTileMaterial)
		FieldHISM->SetMaterial(0, HexTileMaterial);
	FieldHISM->SetOverlayMaterial(nullptr);
	if (BenchTileMesh)
	{
		BenchHISM->SetStaticMesh(BenchTileMesh);
		EnemyHISM->SetStaticMesh(BenchTileMesh);
	}
		
	if (BenchTileMaterial)
	{
		BenchHISM->SetMaterial(0, BenchTileMaterial);
		EnemyHISM->SetMaterial(0, BenchTileMaterial);
	}
	BenchHISM->SetOverlayMaterial(nullptr);
	EnemyHISM->SetOverlayMaterial(nullptr);
	
	Field_InstanceToXY.Reset();
	Bench_InstanceToXY.Reset();
	Enemy_InstanceToXY.Reset();

	// 필드 인스턴스
	for (int32 i = 0; i < FieldTiles.Num(); ++i)
	{
		const int32 InstanceIndex = FieldHISM->AddInstance(FieldTiles[i].WorldTransform, true);
		if (InstanceIndex != INDEX_NONE)
		{
			Field_InstanceToXY.Add(FIntPoint(i,0));
		}
	}

	// 벤치 인스턴스
	for (int32 i = 0; i < BenchTiles.Num(); ++i)
	{
		const int32 InstanceIndex = BenchHISM->AddInstance(BenchTiles[i].WorldTransform, true);
		if (InstanceIndex != INDEX_NONE)
		{
			Bench_InstanceToXY.Add(FIntPoint(i,0));
		}
	}

	// 적 벤치 인스턴스
	for (int32 i = 0; i < EnemyTiles.Num(); ++i)
	{
		const int32 InstanceIndex = EnemyHISM->AddInstance(EnemyTiles[i].WorldTransform, true);
		if (InstanceIndex != INDEX_NONE)
		{
			Enemy_InstanceToXY.Add(FIntPoint(i,0));
		}
	}

	FieldHISM->BuildTreeIfOutdated(true,true);
	BenchHISM->BuildTreeIfOutdated(true,true);
	EnemyHISM->BuildTreeIfOutdated(true,true);
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


void APCCombatBoard::OnHism(bool bOn) const
{
	if (!FieldHISM && !BenchHISM)
		return;
	if (bOn)
	{
		FieldHISM->SetOverlayMaterial(HexTileOverlayMaterial);
		BenchHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	}
	else
	{
		FieldHISM->SetOverlayMaterial(nullptr);
		BenchHISM->SetOverlayMaterial(nullptr);
	}
}

void APCCombatBoard::OnEnemyHism(bool bEnemySide) const
{
	if (!EnemyHISM)
		return;

	if (bEnemySide)
	{
		EnemyHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	}
	else
	{
		EnemyHISM->SetOverlayMaterial(nullptr);
	}
}

FTransform APCCombatBoard::GetPlayerSeatTransform() const
{
	return PlayerSeatAnchor ? PlayerSeatAnchor->GetComponentTransform() : GetActorTransform();
}

FTransform APCCombatBoard::GetEnemySeatTransform() const
{
	return EnemySeatAnchor ? EnemySeatAnchor->GetComponentTransform() : GetActorTransform();
}

void APCCombatBoard::ApplyClientHomeView()
{
	if (!SpringArm)
		return;
	SpringArm->SetRelativeLocation(HomeCam_LocPreset);
	SpringArm->SetRelativeRotation(HomeCam_RocPreset);
}

void APCCombatBoard::ApplyClientMirrorView()
{
	if (!SpringArm)
		return;
	SpringArm->SetRelativeLocation(BattleCameraChangeLocation);
	SpringArm->SetRelativeRotation(BattleCameraChangeRotation);
}

APCBaseUnitCharacter* APCCombatBoard::GetUnitAt(int32 Y, int32 X) const
{
	return TileManager ? TileManager->GetFieldUnit(Y, X) : nullptr;
}

FVector APCCombatBoard::GetFieldUnitLocation(APCBaseUnitCharacter* InUnit) const
{
	return TileManager ? TileManager->GetFieldUnitLocation(InUnit) : FVector::ZeroVector;
}

FIntPoint APCCombatBoard::GetFieldUnitPoint(APCBaseUnitCharacter* InUnit) const
{
	return TileManager ? TileManager->GetFieldUnitGridPoint(InUnit) : FIntPoint::NoneValue;
}

FVector APCCombatBoard::GetTileWorldLocation(int32 Y, int32 X) const
{
	return TileManager ? TileManager->GetTileWorldPosition(Y, X) : FVector::ZeroVector;
}

APCBaseUnitCharacter* APCCombatBoard::GetBenchUnitAt(int32 BenchIndex) const
{
	return TileManager ? TileManager->GetBenchUnit(BenchIndex) : nullptr;
}

FVector APCCombatBoard::GetBenchWorldLocation(int32 BenchIndex) const
{
	return TileManager ? TileManager->GetBenchWorldPosition(BenchIndex) : FVector::ZeroVector;
}

bool APCCombatBoard::IsInRange(int32 Y, int X) const
{
	return TileManager ? TileManager->IsInRange(Y, X) : false;
}

bool APCCombatBoard::IsTileFree(int32 Y, int32 X) const
{
	return TileManager ? TileManager->IsTileFree(Y, X) : false;
}

bool APCCombatBoard::CanUse(int32 Y, int32 X, const APCBaseUnitCharacter* InUnit)
{
	return TileManager ? TileManager->CanUse(Y, X, InUnit) : false;
}

bool APCCombatBoard::HasAnyReservation(const APCBaseUnitCharacter* InUnit)
{
	return TileManager ? TileManager->HasAnyReservation(InUnit) : false;
}

bool APCCombatBoard::SetTileState(int32 Y, int32 X, APCBaseUnitCharacter* InUnit, ETileAction Action)
{
	return TileManager ? TileManager->SetTileState(Y, X, InUnit, Action) : false;
}

void APCCombatBoard::ClearAllForUnit(APCBaseUnitCharacter* InUnit)
{
	if (TileManager)
	{
		TileManager->ClearAllForUnit(InUnit);
	}
}