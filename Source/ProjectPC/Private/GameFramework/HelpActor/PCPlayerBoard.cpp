// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCPlayerBoard.h"

#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Component/PCSynergyComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/Board/PCBoardWidget.h"


APCPlayerBoard::APCPlayerBoard()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(SceneRoot);

	PlayerFieldHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FieldHISM"));
	PlayerFieldHISM->SetupAttachment(SceneRoot);
	PlayerFieldHISM->SetMobility(EComponentMobility::Movable);
	PlayerFieldHISM->NumCustomDataFloats = 4;

	PlayerBenchHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BenchHIS"));
	PlayerBenchHISM->SetupAttachment(SceneRoot);
	PlayerBenchHISM->SetMobility(EComponentMobility::Movable);
	PlayerBenchHISM->NumCustomDataFloats = 4;

	CapacityWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("CapacityWidgetComp"));
	CapacityWidgetComp->SetupAttachment(SceneRoot);
	CapacityWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	CapacityWidgetComp->SetVisibility(false);
}


void APCPlayerBoard::QuickSetUp()
{
	CreatePlayerField();
	CreatePlayerBench();
	BuildHISM();
}

void APCPlayerBoard::OnHISM(bool bIsOn, bool bIsBattle)
{
	if (!PlayerFieldHISM || !PlayerBenchHISM) return;

	if (!bIsOn)
	{
		PlayerFieldHISM->SetOverlayMaterial(nullptr);
		PlayerBenchHISM->SetOverlayMaterial(nullptr);
		return;
	}

	// bIsOn == true
	if (bIsBattle)
	{
		// 전투 중엔 오버레이 끄고 싶다면(원칙만 유지)
		PlayerFieldHISM->SetOverlayMaterial(nullptr);
		PlayerBenchHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	}
	else
	{
		PlayerFieldHISM->SetOverlayMaterial(FieldTileOverlayMaterial);
		PlayerBenchHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	}
}

void APCPlayerBoard::BuildHISM()
{
	PlayerFieldHISM->ClearInstances();
	PlayerBenchHISM->ClearInstances();

	if (FieldTileMesh && FieldTileMaterial)
	{
		PlayerFieldHISM->SetStaticMesh(FieldTileMesh);
		PlayerFieldHISM->SetMaterial(0, FieldTileMaterial);
		PlayerFieldHISM->SetOverlayMaterial(nullptr);
	}
	if (BenchTileMesh && BenchTileMaterial)
	{
		PlayerBenchHISM->SetStaticMesh(BenchTileMesh);
		PlayerBenchHISM->SetMaterial(0, BenchTileMaterial);
		PlayerBenchHISM->SetOverlayMaterial(nullptr);
	}

	// ── 서버에서만 원본 배열을 갱신
	if (HasAuthority())
	{
		FieldLocs.SetNum(PlayerField.Num());
		for (int32 i=0;i<PlayerField.Num();++i)
			FieldLocs[i] = ToWorld(SceneRoot,PlayerField[i].Position);

		BenchLocs.SetNum(PlayerBench.Num());
		for (int32 i=0;i<PlayerBench.Num();++i)
			BenchLocs[i] = ToWorld(SceneRoot, PlayerBench[i].Position);

		// 서버도 즉시 재구성(클라는 OnRep에서)
		RebuildHISM_FromArrays();
	}
	else
	{
		// 클라에서 직접 BuildHISM()을 호출하는 경우는 드묾.
		// 그래도 혹시를 대비해 배열이 차있으면 재구성
		RebuildHISM_FromArrays();
	}
}

void APCPlayerBoard::BeginPlay()
{
	Super::BeginPlay();

	QuickSetUp();
	
	if (HasAuthority())
	{
		CurUnits = CountFieldUnits();
	}

	RefreshCapacityWidget();
}

void APCPlayerBoard::OnRep_FieldLocs()
{
	RebuildHISM_FromArrays();
}

void APCPlayerBoard::OnRep_BenchLocs()
{
	RebuildHISM_FromArrays();
}

void APCPlayerBoard::RebuildHISM_FromArrays()
{
	PlayerFieldHISM->ClearInstances();
	for (const FVector& P : FieldLocs)
	{
		const FTransform LocalT(FRotator::ZeroRotator, FVector(P), FVector::OneVector);
		PlayerFieldHISM->AddInstance(LocalT, /*bWorldSpace=*/false); // 로컬로 추가!
	}

	PlayerBenchHISM->ClearInstances();
	for (const FVector& P : BenchLocs)
	{
		const FTransform LocalT(FRotator::ZeroRotator, FVector(P), FVector::OneVector);
		PlayerBenchHISM->AddInstance(LocalT, /*bWorldSpace=*/false);
	}

	PlayerFieldHISM->BuildTreeIfOutdated(true,true);
	PlayerBenchHISM->BuildTreeIfOutdated(true,true);
}

void APCPlayerBoard::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCPlayerBoard, FieldLocs)
	DOREPLIFETIME(APCPlayerBoard, BenchLocs)
	DOREPLIFETIME_CONDITION(APCPlayerBoard, CurUnits, COND_OwnerOnly)
	DOREPLIFETIME_CONDITION(APCPlayerBoard, MaxUnits, COND_OwnerOnly)
}

#if WITH_EDITOR
void APCPlayerBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	QuickSetUp();
	BuildHISM();
}
#endif

void APCPlayerBoard::CreatePlayerField()
{
	PlayerField.SetNum(Rows * Cols);

	const float R = FieldTileWidthX * 0.5f;
	const float Xs = 1.5f * R;
	const float Ys = FMath::Sqrt(3.f) * R;
	const FVector BaseLocal(FirstFieldLoc.X, FirstFieldLoc.Y, 0);

	for (int32 r=0; r<Rows; ++r)
	{
		const int32 rLocal = bRowZeroAtBottom ? r : (Rows-1-r);
		for (int32 c=0; c<Cols; ++c)
		{
			const int32 i = c*Rows + r;
			const float x = c * Xs;
			const float y = (rLocal + ((c&1)? OddColumRowShift : 0.f)) * Ys;

			PlayerField[i].UnitIntPoint = FIntPoint(c,r);
			PlayerField[i].Position     = BaseLocal + FVector(x,y,0); // ← 로컬 저장
			PlayerField[i].bIsField     = true;
			PlayerField[i].Unit         = nullptr;
		}
	}
}

void APCPlayerBoard::CreatePlayerBench()
{
	const int32 N = FMath::Max(0, BenchSize);
	PlayerBench.SetNum(N);

	const FVector BaseLocal(FirstBenchLoc.X, FirstBenchLoc.Y, 0);
	for (int32 i=0; i<N; ++i)
	{
		PlayerBench[i].Position = BaseLocal + FVector(0.f, i*BenchStepLocalY, 0); // 로컬
		PlayerBench[i].bIsField = false;
		PlayerBench[i].Unit = nullptr;
	}
}

bool APCPlayerBoard::IsInRange(int32 Y, int32 X) const
{
	return (Y >= 0 && Y < Cols && X >= 0 && X < Rows);
}

APCBaseUnitCharacter* APCPlayerBoard::GetFieldUnit(int32 Y, int32 X) const
{
	const int32 i = IndexOf(Y,X);
	return PlayerField.IsValidIndex(i) ? PlayerField[i].Unit : nullptr;
}

APCBaseUnitCharacter* APCPlayerBoard::GetBenchUnit(int32 LocalBenchIndex) const
{
	return PlayerBench.IsValidIndex(LocalBenchIndex) ? PlayerBench[LocalBenchIndex].Unit : nullptr;
}

int32 APCPlayerBoard::GetBenchUnitIndex(APCBaseUnitCharacter* Unit) const
{
	if (!Unit)
		return INDEX_NONE;
	
	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		if (PlayerBench[i].Unit == Unit)
			return i;
	}
	return INDEX_NONE;
}

int32 APCPlayerBoard::GetFieldUnitIndex(APCBaseUnitCharacter* Unit) const
{
	if (!Unit)
		return INDEX_NONE;
	for (int32 i = 0; i < PlayerField.Num(); ++i)
	{
		if (PlayerField[i].Unit == Unit)
			return i;
	}

	return INDEX_NONE;
}

FIntPoint APCPlayerBoard::GetFieldUnitGridPoint(APCBaseUnitCharacter* Unit) const
{
	if (!Unit) return FIntPoint::NoneValue;

	for (int32 y = 0; y < Cols; ++y)
	{
		for (int32 x = 0; x < Rows; ++x)
		{
			int32 i = IndexOf(y,x);
			if (PlayerField[i].Unit == Unit)
			{
				return PlayerField[i].UnitIntPoint;
			}
		}
	}
	return FIntPoint::NoneValue;
}

bool APCPlayerBoard::IsTileFree(int32 Y, int32 X) const
{
	const int32 i = IndexOf(Y,X);
	return PlayerField.IsValidIndex(i) && PlayerField[i].Unit == nullptr;
}

int32 APCPlayerBoard::GetFirstEmptyBenchIndex() const
{
	for (int32 i=0;i<PlayerBench.Num();++i)
		if (PlayerBench[i].Unit == nullptr) return i;
	return INDEX_NONE;
}

TArray<APCBaseUnitCharacter*> APCPlayerBoard::GetAllUnitByTag(FGameplayTag UnitTag, int32 TeamSeat)
{
	TArray<APCBaseUnitCharacter*> Out;
	if (!UnitTag.IsValid()) return Out;

	auto AddIf = [&](APCBaseUnitCharacter* Unit)
	{
		if (IsValid(Unit) && Unit->GetUnitTag().IsValid() && Unit->GetUnitTag().MatchesTag(UnitTag) && Unit->GetTeamIndex() == TeamSeat)
		{
			Out.AddUnique(Unit);
		}
	};

	for (const auto& T : PlayerField)
	{
		AddIf(T.Unit);
	}

	for (const auto& T : PlayerBench)
	{
		AddIf(T.Unit);
	}

	return Out;
}

TArray<APCBaseUnitCharacter*> APCPlayerBoard::GetFieldUnitByTag(FGameplayTag UnitTag)
{
	TArray<APCBaseUnitCharacter*> FieldUnit;
	if (!UnitTag.IsValid())
		return FieldUnit;

	auto AddIfField = [&](APCBaseUnitCharacter* Unit)
	{
		if (!IsValid(Unit)) return;

		if (Unit->GetUnitTag().IsValid() && Unit->GetUnitTag().MatchesTag(UnitTag) && Unit->GetTeamIndex() == PlayerIndex)
		{
			if (!FieldUnit.Contains(Unit))
			{
				FieldUnit.Add(Unit);
			}
		}
	};
	
	for (const auto& T : PlayerField)
	{
		AddIfField(T.Unit);
	}

	return FieldUnit;
}

TArray<APCBaseUnitCharacter*> APCPlayerBoard::GetBenchUnitByTag(FGameplayTag UnitTag, int32 TeamSeat)
{
	TArray<APCBaseUnitCharacter*> BenchUnit;
	if (!UnitTag.IsValid())
		return BenchUnit;

	auto AddIfBench = [&](APCBaseUnitCharacter* Unit)
	{
		if (!IsValid(Unit)) return;

		if (Unit->GetUnitTag().IsValid() && Unit->GetUnitTag().MatchesTag(UnitTag) && Unit->GetTeamIndex() == TeamSeat)
		{
			if (!BenchUnit.Contains(Unit))
			{
				BenchUnit.Add(Unit);
			}
		}
	};
	
	for (const auto& T : PlayerBench)
	{
		AddIfBench(T.Unit);
	}

	return BenchUnit;
}

TArray<FGameplayTag> APCPlayerBoard::GetAllBenchUnitTag()
{
	TArray<FGameplayTag> Out;

	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		if (!PlayerBench[i].IsEmpty())
		{
			FGameplayTag UnitTag = PlayerBench[i].Unit->GetUnitTag();
			Out.AddUnique(UnitTag);
		}
	}

	return Out;
}

bool APCPlayerBoard::EnsureExclusive(APCBaseUnitCharacter* Unit)
{
	if (!Unit) return false;
	if (auto P = GetFieldUnitIndex(Unit); P != INDEX_NONE)
	{
		PlayerField[P].Unit = nullptr;
		return true;
	}
	if (auto bi = GetBenchUnitIndex(Unit); bi != INDEX_NONE)
	{
		ensure(PlayerBench.IsValidIndex(bi));
		PlayerBench[bi].Unit = nullptr;
		return true;
	}
	
	return false;
}

bool APCPlayerBoard::PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit)
{
	if (!HasAuthority()) return false;
	
	APCPlayerState* PCPlayerState = ResolvePlayerState();
	
	if (!PCPlayerState || !Unit || !IsInRange(Y,X)) return false;
	
	const int32 i = IndexOf(Y,X);
	const bool bOccupied = PlayerField.IsValidIndex(i) && IsValid(PlayerField[i].Unit);
	const int32 Future = CountFieldUnits() + (bOccupied ? 0 : 1);

	if (Future > MaxUnits)
	{
		return false;
	}
	
	EnsureExclusive(Unit);
	PlayerField[i].Unit = Unit;
	Unit->ChangedOnTile(true);

	const FVector World = ToWorld(SceneRoot, PlayerField[i].Position);
	FVector TWorld = FVector(World.X, World.Y, 50.f);
	Unit->TeleportTo(TWorld, Unit->GetActorRotation(), false, true);
	
	if (SpawnEffect) 
	{
		if (auto* PC = Cast<APCCombatPlayerController>(PCPlayerState->GetPlayerController()))
		{
			PC->Client_PlaceFX(SpawnEffect,World);
		}
	}

	RecountAndPushToWidget_Server();
	return true;
}

bool APCPlayerBoard::PlaceUnitOnBench(int32 LocalBenchIndex, APCBaseUnitCharacter* Unit)
{
	if (!HasAuthority()) return false;
	
	APCPlayerState* PCPlayerState = ResolvePlayerState();
		
	if (!PCPlayerState || !Unit || !PlayerBench.IsValidIndex(LocalBenchIndex)) return false;
	
	EnsureExclusive(Unit);
	PlayerBench[LocalBenchIndex].Unit = Unit;
	Unit->ChangedOnTile(false);
	
	const FVector World = ToWorld(SceneRoot, PlayerBench[LocalBenchIndex].Position);
	const FRotator ActorRot = GetActorRotation();
	Unit->SetActorLocationAndRotation(FVector(World.X,World.Y,World.Z+50.f),ActorRot,false,nullptr);

	// ✅ 여기서 나이아가라 이펙트 생성
	if (SpawnEffect) // UNiagaraSystem* 타입 멤버
	{
		if (auto* PC = Cast<APCCombatPlayerController>(PCPlayerState->GetPlayerController()))
		{
			PC->Client_PlaceFX(SpawnEffect,World);
		}
	}

	return true;
}

bool APCPlayerBoard::RemoveFromField(int32 FieldIndex)
{
	if (!PlayerField.IsValidIndex(FieldIndex)) return false;
	PlayerField[FieldIndex].Unit = nullptr;
	if (HasAuthority())
	{
		RecountAndPushToWidget_Server();
	}
	
	return true;
}

bool APCPlayerBoard::RemoveFromBench(int32 LocalBenchIndex)
{
	if (!PlayerBench.IsValidIndex(LocalBenchIndex)) return false;
	PlayerBench[LocalBenchIndex].Unit = nullptr;
	return true;
}

bool APCPlayerBoard::RemoveFromBoard(APCBaseUnitCharacter* Unit)
{
	if (!Unit) return false;
	if (auto p = GetFieldUnitIndex(Unit); p != INDEX_NONE)
		return RemoveFromField(p); 
	if (auto bi = GetBenchUnitIndex(Unit); bi != INDEX_NONE)
		return RemoveFromBench(bi);
	
	return false;
}

bool APCPlayerBoard::Swap(APCBaseUnitCharacter* A, APCBaseUnitCharacter* B)
{
	if (!A || !B || A==B) return false;

	const auto PA = GetFieldUnitGridPoint(A); // (Y,X)
	const auto PB = GetFieldUnitGridPoint(B); // (Y,X)
	const int32 PAIndex = GetFieldUnitIndex(A);
	const int32 PBIndex = GetFieldUnitIndex(B);
	const int32 BA = GetBenchUnitIndex(A);
	const int32 BB = GetBenchUnitIndex(B);

	if (PA != FIntPoint::NoneValue && PB != FIntPoint::NoneValue)
	{
		PlayerField[PAIndex].Unit = B;
		PlaceUnitOnField(PA.X, PA.Y, B);
		PlayerField[PBIndex].Unit = A;
		PlaceUnitOnField(PB.X, PB.Y, A);
		return true;
	}

	if (BA != INDEX_NONE && BB != INDEX_NONE)
	{
		PlayerBench[BA].Unit = B;
		PlaceUnitOnBench(BA,B);
		PlayerBench[BB].Unit = A;
		PlaceUnitOnBench(BB,A);
		return true;
	}

	if (PA != FIntPoint::NoneValue && BB != INDEX_NONE)
	{
		PlayerField[PAIndex].Unit = B; // FIX
		PlaceUnitOnField(PA.X, PA.Y, B);
		PlayerBench[BB].Unit = A;
		PlaceUnitOnBench(BB,A);
		return true;
	}

	if (PB != FIntPoint::NoneValue && BA != INDEX_NONE)
	{
		PlayerField[PBIndex].Unit = A;
		PlaceUnitOnField(PB.X, PB.Y, A);
		PlayerBench[BA].Unit = B;
		PlaceUnitOnBench(BA,B);
		return true;
	}

	return false;
}

namespace
{
	// 제곱거릴 헬퍼
	FORCEINLINE float Dist2_2D(const FVector&A, const FVector&B)
	{
		const float dx = A.X - B.X;
		const float dy = A.Y - B.Y;
		return dx*dx + dy*dy;
	}
}

bool APCPlayerBoard::WorldToField(const FVector& World, int32& OutY, int32& OutX, float MaxSnapDist) const
{
	OutY = INDEX_NONE;
	OutX = INDEX_NONE;
	if (PlayerField.Num() <= 0 || Rows <= 0 || Cols <= 0) return false;

	float BestD2 = TNumericLimits<float>::Max();
	int32 BestY = INDEX_NONE;
	int32 BestX = INDEX_NONE;

	for (int32 y = 0; y < Cols; ++y)
	{
		for (int32 x = 0; x < Rows; ++x)
		{
			const int32 i = IndexOf(y, x);
			if (!PlayerField.IsValidIndex(i)) continue;

			const FVector TileW = ToWorld(SceneRoot, PlayerField[i].Position); // 로컬→월드
			const float d2 = FVector::DistSquared2D(TileW, World);
			if (d2 < BestD2) { BestD2 = d2; BestY = y; BestX = x; }
		}
	}

	if (BestY == INDEX_NONE) return false;

	const float DefSnap = (FieldTileWidthX > 0.f) ? (FieldTileWidthX * 0.6f) : 120.f;
	const float Snap    = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;
	if (BestD2 > Snap * Snap) return false;

	OutY = BestY; OutX = BestX;
	return true;
}

bool APCPlayerBoard::WorldToBench(const FVector& World, int32& OutLocalBenchIndex, float MaxSnapDist) const
{
	OutLocalBenchIndex = INDEX_NONE;
	const int32 N = BenchSize;

	float BestD2 = TNumericLimits<float>::Max();
	int32 BestIdx = INDEX_NONE;

	auto try_slot = [&](int32 Local, const FPlayerTile& T)
	{
		const FVector TileW = ToWorld(SceneRoot, T.Position); // 로컬→월드
		const float d2 = FVector::DistSquared2D(TileW, World);
		if (d2 < BestD2) { BestD2 = d2; BestIdx = Local; }
	};

	for (int32 i = 0; i < N; ++i)
		if (PlayerBench.IsValidIndex(i)) try_slot(i, PlayerBench[i]);

	if (BestIdx == INDEX_NONE) return false;

	const float DefSnap = (FieldTileWidthX > 0.f) ? (FieldTileWidthX * 0.6f) : 120.f;
	const float Snap    = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;
	if (BestD2 > Snap * Snap) return false;

	OutLocalBenchIndex = BestIdx;
	return true;
}

bool APCPlayerBoard::WorldAnyTile(const FVector& World, bool bPreferField, bool& bOutIsField, int32& OutY, int32& OutX,
	int32& OutLocalBenchIndex, FVector& OutSnapPos, float MaxSnapField, float MaxSnapBench, bool bRequireUnit) const
{
	bOutIsField = false;
    OutY = OutX = INDEX_NONE;
    OutLocalBenchIndex = INDEX_NONE;
    OutSnapPos = World;

    int32 Y = INDEX_NONE, X = INDEX_NONE, LocalBench = INDEX_NONE;
    const bool bFieldHit = WorldToField(World, Y, X, MaxSnapField);
    const bool bBenchHit = WorldToBench(World, LocalBench, MaxSnapBench);
    if (!bFieldHit && !bBenchHit) return false;

    // 필드 후보
    bool bFieldCandidate = false;
    FVector PFieldW = FVector::ZeroVector; // 월드 좌표
    if (bFieldHit)
    {
        const int32 Idx = IndexOf(Y, X);
        if (PlayerField.IsValidIndex(Idx))
        {
            const bool bHasUnit = (PlayerField[Idx].Unit != nullptr);
            if (!bRequireUnit || bHasUnit)
            {
                bFieldCandidate = true;
                PFieldW = ToWorld(SceneRoot, PlayerField[Idx].Position); // 로컬→월드
            }
        }
    }

    // 벤치 후보
    bool bBenchCandidate = false;
    FVector PBenchW = FVector::ZeroVector; // 월드 좌표
    if (bBenchHit && PlayerBench.IsValidIndex(LocalBench))
    {
        const bool bHasUnit = (PlayerBench[LocalBench].Unit != nullptr);
        if (!bRequireUnit || bHasUnit)
        {
            bBenchCandidate = true;
            PBenchW = ToWorld(SceneRoot, PlayerBench[LocalBench].Position); // 로컬→월드
        }
    }

    if (!bFieldCandidate && !bBenchCandidate) return false;

    if (bFieldCandidate && bBenchCandidate)
    {
        const float df2 = FVector::DistSquared2D(PFieldW, World);
        const float db2 = FVector::DistSquared2D(PBenchW, World);
        constexpr float TieBias = 1.02f;
        const bool ChooseField = bPreferField ? (df2 <= db2 * TieBias) : (df2 < db2);

        if (ChooseField) { bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PFieldW; }
        else             { bOutIsField = false; OutLocalBenchIndex = LocalBench; OutSnapPos = PBenchW; }
        return true;
    }

    if (bFieldCandidate) { bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PFieldW;  return true; }
    else                 { bOutIsField = false; OutLocalBenchIndex = LocalBench; OutSnapPos = PBenchW; return true; }
}

void APCPlayerBoard::AttachToCombatBoard(APCCombatBoard* CombatBoardToFollow, bool bIsGuest)
{
	if (!IsValid(CombatBoardToFollow)) return;

	SaveHomeTransform = GetActorTransform();

	const FTransform BoardXf = CombatBoardToFollow->GetActorTransform();
	const FRotator  NewRot(BoardXf.Rotator().Pitch,
						   FMath::UnwindDegrees(BoardXf.Rotator().Yaw + AttachYawOffsetDeg),
						   BoardXf.Rotator().Roll);

	SetActorLocationAndRotation(BoardXf.GetLocation(), NewRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 인스턴스는 로컬이므로, 보드가 이동하면 같이 따라감. 그래도 재빌드하면 안전.
	ResnapBenchUnitsToBoard(true);
	BuildHISM();
}

void APCPlayerBoard::DetachFromCombatBoard()
{
	// 원래 홈 위치/회전으로 복귀
	if (!SaveHomeTransform.Equals(FTransform::Identity))
	{
		SetActorTransform(SaveHomeTransform, /*bSweep=*/false, /*OutHit=*/nullptr, ETeleportType::TeleportPhysics);
	}
	
	// 벤치 유닛들을 현재 PlayerBoard 기준 벤치 타일 위치로 재스냅
	ResnapBenchUnitsToBoard(false);
	BuildHISM();
}

void APCPlayerBoard::ResnapBenchUnitsToBoard(bool bIsBattle)
{
	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		if (APCBaseUnitCharacter* Unit = PlayerBench[i].Unit)
		{
			const FVector Loc = ToWorld(SceneRoot,PlayerBench[i].Position);
			FVector NewLoc =FVector(Loc.X, Loc.Y, Loc.Z + 30);
			FRotator Rot = FRotator(0,0,0);

			if (bIsBattle)
			{
				Rot = FRotator(0,180,0);
			}
			
			Unit->SetActorLocationAndRotation(NewLoc , Rot, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void APCPlayerBoard::MakeSnapshot(FPlayerBoardSnapshot& Out) const
{
	Out.FieldUnits.SetNum(Rows*Cols);
	for (int32 i=0;i<Out.FieldUnits.Num();++i) Out.FieldUnits[i] = PlayerField.IsValidIndex(i) ? PlayerField[i].Unit : nullptr;
}

void APCPlayerBoard::ApplySnapshot(const FPlayerBoardSnapshot& In)
{
	const int32 NField = Rows*Cols;
	for (int32 i=0;i<NField && i<In.FieldUnits.Num(); ++i)
		if (PlayerField.IsValidIndex(i)) PlayerField[i].Unit = In.FieldUnits[i];
}

FVector APCPlayerBoard::GetFieldWorldPos(int32 Y, int32 X) 
{
	const int32 i = IndexOf(Y,X);
	return PlayerField.IsValidIndex(i) ? ToWorld(SceneRoot, PlayerField[i].Position) : FVector::ZeroVector;
}

FVector APCPlayerBoard::GetBenchWorldPos(int32 LocalBenchIndex) 
{
	return PlayerBench.IsValidIndex(LocalBenchIndex) ? ToWorld(SceneRoot, PlayerBench[LocalBenchIndex].Position) : FVector::ZeroVector;
}

void APCPlayerBoard::OnRep_FieldCount()
{
	RefreshCapacityWidget();
}

void APCPlayerBoard::OnRep_MaxUnits()
{
	RefreshCapacityWidget();
}

void APCPlayerBoard::RefreshCapacityWidget() const
{
	if (auto* Widget = Cast<UPCBoardWidget>(CapacityWidgetComp->GetUserWidgetObject()))
	{
		Widget->SetValues(CurUnits,MaxUnits);
	}
}

int32 APCPlayerBoard::CountFieldUnits() const
{
	int32 Count = 0;
	for (const auto& T : PlayerField)
	{
		if (IsValid(T.Unit))
		{
			++Count;
		}
	}
	
	return Count;
}

void APCPlayerBoard::RecountAndPushToWidget_Server()
{
	if (!HasAuthority())
		return;
	CurUnits = CountFieldUnits();
	OnRep_FieldCount();
}

void APCPlayerBoard::SetCapacityWidgetVisible_Implementation(const FGameplayTag& GameState)
{
	const bool bVisible = GameState == GameStateTags::Game_State_NonCombat;

	if (CapacityWidgetComp)
	{
		CapacityWidgetComp->SetVisibility(bVisible);
		
	}
}

void APCPlayerBoard::OnLevelChanged(const FOnAttributeChangeData& Data)
{
	SyncMaxFromLevel();
}

void APCPlayerBoard::PlayerBoardDelegate()
{
	if (UAbilitySystemComponent* ASC = ResolveASC())
	{
		if (const UPCPlayerAttributeSet* Atr = ResolveSet())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Atr->GetPlayerLevelAttribute())
			.AddUObject(this, &ThisClass::OnLevelChanged);
		}
	}

	if (APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
	{
		PCGameState->OnGameStateTagChanged.AddUObject(this,&ThisClass::SetCapacityWidgetVisible);
		PCGameState->OnGameStateTagChanged.AddUObject(this,&ThisClass::OnGameStateChangedForAutoFill);
	}

	SyncMaxFromLevel();
}

void APCPlayerBoard::SyncMaxFromLevel()
{
	int32 Level = 1;
	if (const UPCPlayerAttributeSet* Attr = ResolveSet())
	{
		Level = Attr->GetPlayerLevel();
	}
	
	MaxUnits = Level;
	OnRep_MaxUnits();
}

APCPlayerState* APCPlayerBoard::ResolvePlayerState() const
{
	return OwnerPlayerState;
}

UAbilitySystemComponent* APCPlayerBoard::ResolveASC() const
{
	if (APCPlayerState* PS = ResolvePlayerState())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

const UPCPlayerAttributeSet* APCPlayerBoard::ResolveSet() const
{
	if (APCPlayerState* PCPlayerState = ResolvePlayerState())
	{
		return PCPlayerState->GetAttributeSet();
	}
	
	return nullptr;
}

void APCPlayerBoard::TryAutoFillFromBench_Server()
{
	if (!HasAuthority())
		return;

	const int32 Capacity = MaxUnits;
	int32 Cur = CountFieldUnits();
	if (Cur >= Capacity)
		return;

	while (Cur<Capacity)
	{
		int32 y = INDEX_NONE;
		int32 x = INDEX_NONE;
		if (!FindFirstEmptyField(y,x))
			break;

		const int32 BenchIdx = GetFirstOccupiedBenchIndex();
		if (BenchIdx == INDEX_NONE)
			break;

		APCBaseUnitCharacter* Unit = PlayerBench[BenchIdx].Unit;
		if (!IsValid(Unit))
		{
			PlayerBench[BenchIdx].Unit = nullptr;
			continue;
		}
		
		RemoveFromBench(BenchIdx);
		const bool bPlaced = PlaceUnitOnField(y,x,Unit);		
		if (!bPlaced)
			break;

		++Cur;
	}
}

void APCPlayerBoard::OnGameStateChangedForAutoFill(const FGameplayTag& GameState)
{
	if (!HasAuthority())
		return;

	if (GameState == GameStateTags::Game_State_Combat_Preparation)
	{
		TryAutoFillFromBench_Server();
	}
}


bool APCPlayerBoard::FindFirstEmptyField(int32& OutY, int32& OutX) const
{
	for (int32 y = 0; y < Cols; ++y)
	{
		for (int32 x = 0; x < Rows; ++x)
		{
			if (IsTileFree(y,x))
			{
				OutY = y;
				OutX = x;
				return true;
			}
		}
	}
	OutY = INDEX_NONE;
	OutX = INDEX_NONE;
	return false;
}

int32 APCPlayerBoard::GetFirstOccupiedBenchIndex() const
{
	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		if (PlayerBench[i].Unit)
			return i;
	}

	return INDEX_NONE;
}
