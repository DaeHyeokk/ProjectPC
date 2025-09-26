#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCDragComponent.generated.h"

class APCHeroUnitCharacter;
class APCPreviewHeroActor;
class APCCombatPlayerController;
class APCCombatBoard;
class UPCTileManager;
class APCDragGhost;

UENUM(BlueprintType)
enum class EDragState : uint8
{
    Idle,
    Pending,
    Dragging,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPC_API UPCDragComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPCDragComponent();
    
    // 컨트롤러의 입력 바인딩에서 호출
    bool OnMouse_Pressed(class APCCombatPlayerController* PC);
    void OnMouse_Released(class APCCombatPlayerController* PC);

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === 서버 피드백 수신 (Controller의 Client RPC에서 이쪽으로 포워딩) ===
    void OnServerDragConfirm(bool bOk, int32 DragId, const FVector& StartSnap, APCHeroUnitCharacter* PreviewHero);
    void OnServerDragEndResult(bool bSuccess, const FVector& FinalSnap, int32 DragId, APCHeroUnitCharacter* PreviewHero);

    UFUNCTION()
    bool IsDraggingOrPending(); 
    
private:
    // 상태
    UPROPERTY(Transient) EDragState State = EDragState::Idle;
    UPROPERTY(Transient) int32 LocalDragId = 0;   // 로컬에서 증가
    UPROPERTY(Transient) int32 ActiveDragId = 0;  // 현재 세션 id
    
    // 마지막 전송 위치(불필요한 틱 전송 억제)
    FVector LastSnep = FVector::ZeroVector;

    // 마지막으로 질의한 타일 (중복 질의 억제)
    bool LastQuestion_bIsField = false;
    int32 LastQuestion_Y = INDEX_NONE;
    int32 LastQuestion_X = INDEX_NONE;
    int32 LastQuestion_Bench = INDEX_NONE;

    // 고스트
    UPROPERTY(Transient)
    TWeakObjectPtr<APCPreviewHeroActor> Preview;
    
    void EnsureGhostAt(const FVector& World, APCHeroUnitCharacter* PreviewHero);
    void ShowGhost(const FVector& World, APCHeroUnitCharacter* PreviewHero);
    void HideGhost(APCHeroUnitCharacter* PreviewHero);

    // 커서 히트
    bool CursorHitWorld(APCCombatPlayerController* PC, FVector& OutWorld) const;


protected:
    virtual void BeginPlay() override;
};

