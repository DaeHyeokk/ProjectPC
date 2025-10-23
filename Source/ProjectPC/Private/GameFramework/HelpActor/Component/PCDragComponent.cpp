#include "GameFramework/HelpActor/Component/PCDragComponent.h"

#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Character/Unit/PCPreviewHeroActor.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"

UPCDragComponent::UPCDragComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UPCDragComponent::OnMouse_Pressed(APCCombatPlayerController* PC)
{
    if (!PC || State != EDragState::Idle) return false;

    FVector World;
    if (!CursorHitWorld(PC, World)) return false;

    // 로컬: Pending 표시(회색 등) 후 서버에 시작 요청
    State = EDragState::Pending;
    ActiveDragId = ++LocalDragId;

    EnsureGhostAt(World, nullptr);
    ShowGhost(World);
    
    SetComponentTickEnabled(true);

    // 서버가 타일/유닛을 찾아 승인/거절
    PC->Server_StartDragFromWorld(World, ActiveDragId); // Reliable

    return true;
}

void UPCDragComponent::OnMouse_Released(APCCombatPlayerController* PC)
{
    if (!PC || State == EDragState::Idle) return;

    FVector World;
    if (!CursorHitWorld(PC, World))
        World = LastSnep;

    // 서버에 종료 요청(유효하면 이동 수행)
    PC->Server_EndDrag(World, ActiveDragId); // Reliable

    // 로컬 상태는 일단 종료 처리(서버가 최종 피드백 제공)
    State = EDragState::Idle;
    SetComponentTickEnabled(false);

    HideGhost();
}

void UPCDragComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (State != EDragState::Dragging && State != EDragState::Pending) return;

    auto* PC = Cast<APCCombatPlayerController>(GetOwner());
    if (!PC) return;

    FVector World;
    if (!CursorHitWorld(PC, World)) return;

    FVector Snap = World;
    if (APCPlayerBoard* PlayerBoard = PC->GetPlayerBoard())
    {
        bool bField = false;
        int32 Y = -1;
        int32 X = -1;
        int32 BenchIdx = -1;

        if (PlayerBoard->WorldAnyTile(World, true, bField, Y, X, BenchIdx, Snap))
        {
            EnsureGhostAt(Snap,nullptr);
            ShowGhost(Snap);
            
            const bool bTileChanged = (bField != LastQuestion_bIsField) || (Y != LastQuestion_Y) || (X != LastQuestion_X) ||  (BenchIdx != LastQuestion_Bench);
            if (bTileChanged)
            {
                LastQuestion_bIsField = bField;
                LastQuestion_Y = Y;
                LastQuestion_X = X;
                LastQuestion_Bench = BenchIdx;
                PC->Server_QueryTileUnit(bField, Y, X, BenchIdx);
            }
            return;
        }
    }
    
    ShowGhost(Snap);
    LastSnep = World;
}

void UPCDragComponent::OnServerDragConfirm(bool bOk, int32 DragId, const FVector& StartSnap, APCHeroUnitCharacter* PreviewHero)
{
    if (DragId != ActiveDragId)
        return;
    if (!bOk)
    {
        State = EDragState::Idle;
        SetComponentTickEnabled(false);
        return;
    }
        
    EnsureGhostAt(StartSnap, PreviewHero);
    ShowGhost(StartSnap);
    
    State = EDragState::Dragging;
    LastSnep = StartSnap;
}

void UPCDragComponent::OnServerDragEndResult(bool bSuccess, const FVector& FinalSnap, int32 DragId, APCHeroUnitCharacter* PreviewHero)
{
    if (DragId != ActiveDragId)
        return;
    
    if (bSuccess)
    {
        EnsureGhostAt(FinalSnap, nullptr);
        ShowGhost(FinalSnap);
    }
    
    HideGhost();
    
    State = EDragState::Idle;
    SetComponentTickEnabled(false);
}

bool UPCDragComponent::IsDraggingOrPending()
{
    return State == EDragState::Pending || State == EDragState::Dragging;
}


bool UPCDragComponent::CursorHitWorld(APCCombatPlayerController* PC, FVector& OutWorld) const
{
    FHitResult Hit;
    if (PC && PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit))
    {
        OutWorld = Hit.Location;
        return true;
    }
    OutWorld = FVector::ZeroVector;
    return false;
}

void UPCDragComponent::BeginPlay()
{
    Super::BeginPlay();
    
}

void UPCDragComponent::EnsureGhostAt(const FVector& World, APCHeroUnitCharacter* PreviewHero)
{
    if (Preview.IsValid())
    {
        Preview->UpdateLocation(World);
        return;
    }
    
    if (!Preview.IsValid())
    {
        if (PreviewHero)
        {
            if (UWorld* WorldPtr = GetWorld())
            {
                if (UPCUnitSpawnSubsystem* SubSystem = WorldPtr->GetSubsystem<UPCUnitSpawnSubsystem>())
                {
                    Preview = SubSystem->SpawnPreviewHeroBySourceHero(PreviewHero, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
                    FRotator HeroRot = PreviewHero->GetActorRotation();
                    const FRotator  NewRot(HeroRot.Pitch,FMath::UnwindDegrees(HeroRot.Yaw - 90.f),HeroRot.Roll);
                    Preview->SetActorRotation(NewRot);
                }
            }
        }
    }
    
}

void UPCDragComponent::ShowGhost(const FVector& World)
{
    if (Preview.IsValid())
    {
        Preview->UpdateLocation(World);
    }  
}

void UPCDragComponent::HideGhost()
{
  if (Preview.IsValid())
  {
      Preview->TearDown();
  }

    Preview = nullptr;
}