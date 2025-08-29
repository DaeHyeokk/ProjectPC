// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCombatBoard.h"

#include "Camera/CameraComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Field/FieldSystemNoiseAlgo.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/HelpActor/PCUnitVisual.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Kismet/GameplayStatics.h"


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
}


void APCCombatBoard::BeginPlay()
{
	Super::BeginPlay();
	RebuildAnchors();
	RebuildTilesFromMarkers();

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APCPlayerState* PCPlayerState = PlayerController->GetPlayerState<APCPlayerState>())
		{
			if (PCPlayerState->SeatIndex == BoardSeatIndex)
			{
				BoundPCPlayerState = PCPlayerState;
				BoundHandle = PCPlayerState->OnBoardUpdated.AddLambda([this, PCPlayerState]()
				{
					UpdateBoardFromPlayerState(PCPlayerState);
				});
				UpdateBoardFromPlayerState(PCPlayerState);
			}
		}
	}
	
}

void APCCombatBoard::EndPlay(const EEndPlayReason::Type reason)
{
	if (BoundPCPlayerState.IsValid() && BoundHandle.IsValid())
	{
		BoundPCPlayerState->OnBoardUpdated.Remove(BoundHandle);
	}
	Super::EndPlay(reason);
	
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


bool APCCombatBoard::TryGetTileFromHit(const FHitResult& Hit, int32& OutX, int32& OutY, bool& bBench) const
{
	OutX = -1;
	OutY = -1;
	bBench = false;
	if (Hit.Component.Get() == FieldHISM)
	{
		const int32 HitComponentIdx = Hit.Item;
		if (Field_InstanceToXY.IsValidIndex(HitComponentIdx))
		{
			const FIntPoint XY = Field_InstanceToXY[HitComponentIdx];
			OutX = XY.X;
			OutY = XY.Y;
			bBench = false;
			return true;
		}
	}

	if (Hit.Component.Get() == BenchHISM)
	{
		const int32 HitComponentIdx = Hit.Item;
		if (Bench_InstanceToXY.IsValidIndex(HitComponentIdx))
		{
			const FIntPoint XY = Bench_InstanceToXY[HitComponentIdx];
			OutX = XY.X;
			OutY = XY.Y;
			bBench = true;
			return true;
		}
	}
	return false;
}

void APCCombatBoard::UpdateBoardFromPlayerState(APCPlayerState* PCPlayerState)
{
	if (!PCPlayerState) return;

	TBitArray<> UsedField(false, FieldTiles.Num());
	TBitArray<> UsedBench(false, BenchTiles.Num());

	for (int32 i = 0; i < PCPlayerState->FieldUnit.Num(); ++i)
	{
		const FUnitDataInBoard& UnitDataInBoard = PCPlayerState->FieldUnit[i];
	
		if (!FieldTiles.IsValidIndex(UnitDataInBoard.TileIndex)) continue;
		UsedField[UnitDataInBoard.TileIndex] = true;
		const FTransform& Transform = FieldTiles[UnitDataInBoard.TileIndex].WorldTransform;
		SpawnOrMoveVisual(false, UnitDataInBoard.TileIndex, UnitDataInBoard.UnitID, Transform);
	}

	for (int32 i = 0; i < PCPlayerState->BenchUnit.Num(); ++i)
	{
		const FUnitDataInBoard& UnitDataInBoard = PCPlayerState->BenchUnit[i];
		if (!BenchTiles.IsValidIndex(UnitDataInBoard.TileIndex)) continue;

		UsedBench[UnitDataInBoard.TileIndex] = true;
		const FTransform Transform = BenchTiles[UnitDataInBoard.TileIndex].WorldTransform;
		SpawnOrMoveVisual(true, UnitDataInBoard.TileIndex, UnitDataInBoard.UnitID, Transform);
	}

	GarbageUnusedVisual(false, UsedField);
	GarbageUnusedVisual(true, UsedBench);
}


void APCCombatBoard::SpawnOrMoveVisual(bool bBench, int32 TileIndex, FGameplayTag UnitID, const FTransform& Transform)
{
	const int32 Key = MakeKey(bBench, TileIndex);
	APCUnitVisual* Visual = Visuals.FindRef(Key).Get();

	if (!Visual)
	{
		if (!UnitVisualClass) return;
		Visual = GetWorld()->SpawnActor<APCUnitVisual>(UnitVisualClass, Transform);
		if (Visual)
		{
			Visuals.Add(Key, Visual);
			Visual->Init(UnitID);
		}
		return;
	}

	Visual->SetActorTransform(Transform);
	Visual->SetUnitID(UnitID);
	
}



void APCCombatBoard::GarbageUnusedVisual(bool bBench, const TBitArray<>& Used)
{
	TArray<int32> ToRemove;
	for (const auto& KVP : Visuals)
	{
		const int32 Key = KVP.Key;
		const bool IsBenchKey = (Key >= 1000000);
		if (IsBenchKey != bBench) continue;

		const int32 Idx = Key % 1000000;
		if (!Used.IsValidIndex(Idx) || !Used[Idx])
		{
			if (AActor* Visual = KVP.Value.Get())
			{
				Visual->Destroy();
			}
			ToRemove.Add(Key);
		}
	}

	for (int32 K : ToRemove)
	{
		Visuals.Remove(K);
	}
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




