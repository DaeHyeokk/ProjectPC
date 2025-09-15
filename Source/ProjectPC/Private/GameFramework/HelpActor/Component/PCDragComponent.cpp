#include "GameFramework/HelpActor/Component/PCDragComponent.h"

#include "Components/SplineComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCDragGhost.h"
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

    EnsureGhostAt(World);
    ShowGhost(World, /*bApproved=*/false, /*bValid=*/true);

    LastSentWorld = World;
    LastSendTime  = 0.0;
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
        World = LastSentWorld;

    // 서버에 종료 요청(유효하면 이동 수행)
    PC->Server_EndDrag(World, ActiveDragId); // Reliable

    // 로컬 상태는 일단 종료 처리(서버가 최종 피드백 제공)
    State = EDragState::Idle;
    SetComponentTickEnabled(false);
}

void UPCDragComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (State != EDragState::Dragging) return;

    auto* PC = Cast<APCCombatPlayerController>(GetOwner());
    if (!PC) return;

    FVector World;
    if (!CursorHitWorld(PC, World)) return;

    // 과도 전송 억제: 위치 변화 + 전송레이트 체크
    const double Now = GetWorld()->TimeSeconds;
    const bool bMovedEnough = FVector::DistSquared2D(World, LastSentWorld) > FMath::Square(8.f);
    const bool bRateOK = (Now - LastSendTime) >= (1.0 / SendHz);

    if (bMovedEnough && bRateOK)
    {
        LastSentWorld = World;
        LastSendTime  = Now;
        PC->Server_UpdateDrag(World, ActiveDragId); // Unreliable
    }
}

void UPCDragComponent::OnServerDragConfirm(bool bOk, int32 DragId, const FVector& StartSnap)
{
    if (DragId != ActiveDragId)
        return;
    if (!bOk)
    {
        State = EDragState::Idle;
        SetComponentTickEnabled(false);
        HideGhost();
        return;
    }

    State = EDragState::Dragging;
    EnsureGhostAt(StartSnap);
    ShowGhost(StartSnap, true, true);
}

void UPCDragComponent::OnServerDragHint(const FVector& Snap, bool bValid, int32 DragId)
{
    if (DragId != ActiveDragId || State != EDragState::Dragging)
        return;
    EnsureGhostAt(Snap);
    ShowGhost(Snap, true, bValid);
}

void UPCDragComponent::OnServerDragEndResult(bool bSuccess, const FVector& FinalSnap, int32 DragId)
{
    if (DragId != ActiveDragId)
        return;
    if (bSuccess)
    {
        EnsureGhostAt(FinalSnap);
        ShowGhost(FinalSnap, true, true);
    }
    HideGhost();

    State = EDragState::Idle;
    SetComponentTickEnabled(false);
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

void UPCDragComponent::EnsureGhostAt(const FVector& World)
{
    if (!Ghost.IsValid())
    {
        Ghost = APCDragGhost::SpawnGhost(GetWorld(), World);
    }
    
}

void UPCDragComponent::ShowGhost(const FVector& World, bool bApproved, bool bValid)
{
    EnsureGhostAt(World);
    if (Ghost.IsValid())
    {
        Ghost->UpdateState(true, bApproved && bValid, World);
    }


}


void UPCDragComponent::HideGhost()
{
    if (Ghost.IsValid())
    {
        Ghost->UpdateState(false, false, Ghost->GetActorLocation());
        Ghost->Destroy();
        Ghost = nullptr;
    }
}

