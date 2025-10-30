// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCombatBoard.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Net/UnrealNetwork.h"


APCCombatBoard::APCCombatBoard()
{ 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);
		
	// 카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->bDoCollisionTest = false;
	SpringArm->TargetArmLength = 1000.f;
	

	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SpringArm);
	BoardCamera->FieldOfView = 60.f;

	// Tile Manger
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

	SpringArm->SetRelativeLocation(HomeCam_LocPreset);
	SpringArm->SetRelativeRotation(HomeCam_RocPreset);

	BoardCamera->SetRelativeLocation(HomeCameraOffset);
	BoardCamera->SetRelativeRotation(HomeCameraRotation);

	RebuildAnchors();

	if (TileManager)
	{
		TileManager->QuickSetUp();
		// TileManager->DebugDrawTiles(1e6f, true);
	}
}

void APCCombatBoard::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!TileManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Board %s] TileManager was null. Recreate at runtime."), *GetName());

		TileManager = NewObject<UPCTileManager>(this, TEXT("TileManager_Runtime"));
		// 컴포넌트면 반드시 RegisterComponent 호출
		TileManager->RegisterComponent();
		// 필요시 초기화 코드 호출
	}
	
}

#if WITH_EDITOR
void APCCombatBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildAnchors();
		
}
#endif

USceneComponent* APCCombatBoard::Resolve(const FComponentReference& Ref) const
{
	return Cast<USceneComponent>(Ref.GetComponent(const_cast<APCCombatBoard*>(this)));
}


static int32 NameSuffixToIndex(const FString& Name, const FString& Prefix)
{
	if (!Name.StartsWith(Prefix))
		return -1;
	int32 Index = -1;
	LexTryParseString(Index, *Name.RightChop(Prefix.Len()));
	return Index;
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