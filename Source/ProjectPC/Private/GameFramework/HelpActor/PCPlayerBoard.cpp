// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCPlayerBoard.h"

#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"


APCPlayerBoard::APCPlayerBoard()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlayerFieldHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("FieldHISM"));
	PlayerFieldHISM->SetupAttachment(SceneRoot);
	PlayerFieldHISM->SetMobility(EComponentMobility::Static);
	PlayerFieldHISM->NumCustomDataFloats = 4;

	PlayerBenchHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("BenchHIS"));
	PlayerBenchHISM->SetupAttachment(SceneRoot);
	PlayerBenchHISM->SetMobility(EComponentMobility::Static);
	PlayerBenchHISM->NumCustomDataFloats = 4;

}


void APCPlayerBoard::QuickSetUp()
{
	CreatePlayerField();
	CreatePlayerBench();
	BuildHISM();
}

void APCPlayerBoard::BuildHISM()
{
	PlayerFieldHISM->ClearInstances();
	PlayerBenchHISM->ClearInstances();

	if (FieldTileMesh && FieldTileMaterial && FieldTileOverlayMaterial)
	{
		PlayerFieldHISM->SetStaticMesh(FieldTileMesh);
		PlayerFieldHISM->SetMaterial(0, FieldTileMaterial);
		PlayerFieldHISM->SetOverlayMaterial(FieldTileOverlayMaterial);
	}
	if (BenchTileMesh && BenchTileMaterial && BenchTileOverlayMaterial)
	{
		PlayerBenchHISM->SetStaticMesh(BenchTileMesh);
		PlayerBenchHISM->SetMaterial(0, BenchTileMaterial);
		PlayerBenchHISM->SetOverlayMaterial(BenchTileOverlayMaterial);
	}

	for (int32 i = 0; i < PlayerField.Num(); ++i)
	{
		const FTransform WorldXForm(FRotator::ZeroRotator, PlayerField[i].Position, FVector::OneVector);
		PlayerFieldHISM->AddInstance(WorldXForm, true);
	}

	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		const FTransform worldXForm(FRotator::ZeroRotator, PlayerBench[i].Position, FVector::OneVector);
		PlayerBenchHISM->AddInstance(worldXForm, true);
	}

	PlayerFieldHISM->BuildTreeIfOutdated(true, true);
	PlayerBenchHISM->BuildTreeIfOutdated(true, true);
	
}

void APCPlayerBoard::BeginPlay()
{
	Super::BeginPlay();

	QuickSetUp();
	BuildHISM();
	
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

	const FVector SelfLocation = GetActorLocation();

	const float Radios = FieldTileWidthX * 0.5;
	const float Xs = 1.5f * Radios;
	const float Ys = FMath::Sqrt(3.f) * Radios;

	const FVector Base = SelfLocation + FVector(FirstFieldLoc.X, FirstFieldLoc.Y,0);

	for ( int32 r = 0; r < Rows; ++r )
	{
		const int32 rLocal  = bRowZeroAtBottom ? r : (Rows - 1 - r);

		for (int32 c = 0; c < Cols; ++c)
		{
			const int32 i = c * Rows + r;
			const float x = c * Xs;

			const float RowShift = ((c&1) ? OddColumRowShift : 0.0f);
			const float y = (rLocal + RowShift) * Ys;
			PlayerField[i].UnitIntPoint = FIntPoint(c,r);
			PlayerField[i].Position = Base + FVector(x, y, 0);
			PlayerField[i].bIsField = true;
			PlayerField[i].Unit = nullptr;
		}
	}
}

void APCPlayerBoard::CreatePlayerBench()
{
	const int32 N = FMath::Max(0, BenchSize);
	PlayerBench.SetNum(N);

	const FVector SelfLocation = GetActorLocation();
	const FVector Base = SelfLocation + FVector(FirstBenchLoc.X, FirstBenchLoc.Y,0);

	for (int32 i = 0; i < N; ++i)
	{
		PlayerBench[i].Position = Base + FVector(0.f, i * BenchStepLocalY, 0);
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

FIntPoint APCPlayerBoard::GetFieldUnitGridPoint(APCBaseUnitCharacter* Unit) const
{
	if (!Unit) return FIntPoint::NoneValue;
	for (int32 x = 0; x < Rows; ++x)
	{
		for (int32 y = 0; y < Cols; ++y)
		{
			if (GetFieldUnit(y,x) == Unit)
				return FIntPoint(y,x);
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

		if (Unit->GetUnitTag().IsValid() && Unit->GetUnitTag().MatchesTag(UnitTag) && Unit->GetTeamIndex() == PlayerIndex)
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

bool APCPlayerBoard::EnsureExclusive(APCBaseUnitCharacter* Unit)
{
	if (!Unit) return false;
	if (auto P = GetFieldUnitGridPoint(Unit); P != FIntPoint::NoneValue)
	{
		PlayerField[IndexOf(P.X, P.Y)].Unit = nullptr;
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
	if (!Unit || !IsInRange(Y,X)) return false;
	EnsureExclusive(Unit);
	const int32 i = IndexOf(Y,X);
	PlayerField[i].Unit = Unit;
	Unit->SetActorLocation(PlayerField[i].Position);

	UE_LOG(LogTemp, Warning, TEXT("Place Field Y=%d X=%d i=%d Pos=%s"),
		   Y, X, IndexOf(Y,X), *PlayerField[IndexOf(Y,X)].Position.ToString());
	return true;
}

bool APCPlayerBoard::PlaceUnitOnBench(int32 LocalBenchIndex, APCBaseUnitCharacter* Unit)
{
	if (!Unit || !PlayerBench.IsValidIndex(LocalBenchIndex)) return false;
	EnsureExclusive(Unit);
	PlayerBench[LocalBenchIndex].Unit = Unit;
	Unit->SetActorLocation(PlayerBench[LocalBenchIndex].Position);
	return true;
}

bool APCPlayerBoard::RemoveFromField(int32 Y, int32 X)
{
	const int32 i = IndexOf(Y,X);
	if (!PlayerField.IsValidIndex(i)) return false;
	PlayerField[i].Unit = nullptr;
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
	if (auto p = GetFieldUnitGridPoint(Unit); p != FIntPoint::NoneValue)
		return RemoveFromField(p.Y, p.X); // FIX: (Y,X)
	if (auto bi = GetBenchUnitIndex(Unit); bi != INDEX_NONE)
		return RemoveFromBench(bi);
	return false;
}

bool APCPlayerBoard::Swap(APCBaseUnitCharacter* A, APCBaseUnitCharacter* B)
{
	if (!A || !B || A==B) return false;

	const auto PA = GetFieldUnitGridPoint(A); // (Y,X)
	const auto PB = GetFieldUnitGridPoint(B); // (Y,X)
	const int32 BA = GetBenchUnitIndex(A);
	const int32 BB = GetBenchUnitIndex(B);

	if (PA != FIntPoint::NoneValue && PB != FIntPoint::NoneValue)
	{
		PlayerField[IndexOf(PA.X, PA.Y)].Unit = B;
		PlaceUnitOnField(PA.X, PA.Y, B);
		PlayerField[IndexOf(PB.X, PB.Y)].Unit = A;
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
		PlayerField[IndexOf(PA.X, PA.Y)].Unit = B; // FIX
		PlaceUnitOnField(PA.X, PA.Y, B);
		PlayerBench[BB].Unit = A;
		PlaceUnitOnBench(BB,A);
		return true;
	}

	if (PB != FIntPoint::NoneValue && BA != INDEX_NONE)
	{
		PlayerField[IndexOf(PB.X, PB.Y)].Unit = A;
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

	if (PlayerField.Num() <= 0 || Rows <= 0 || Cols <= 0)
		return false;

	float BestD2 = TNumericLimits<float>::Max();
	int32 BestY = INDEX_NONE;
	int32 BestX = INDEX_NONE;

	// 전체 스캔 (Rows*Cols), 보드 크기가 작으니 충분히 빠름
	for (int32 y = 0; y < Cols; ++y)
	{
		for (int32 x = 0; x < Rows; ++x)
		{
			const int32 i = IndexOf(y, x);
			if (!PlayerField.IsValidIndex(i)) continue;

			const float d2 = Dist2_2D(PlayerField[i].Position, World);
			if (d2 < BestD2)
			{
				BestD2 = d2;
				BestY = y;
				BestX = x;
			}
		}
	}

	if (BestY == INDEX_NONE)
		return false;

	// 스냅 임계값: 제곱거리끼리 비교!
	const float DefSnap = (FieldTileWidthX > 0.f) ? (FieldTileWidthX * 0.6f) : 120.f;
	const float Snap = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;

	if (BestD2 > (Snap * Snap))
		return false;

	OutY = BestY;
	OutX = BestX;
	
	return true;
	
}

bool APCPlayerBoard::WorldToBench(const FVector& World, int32& OutLocalBenchIndex, float MaxSnapDist) const
{
	OutLocalBenchIndex = INDEX_NONE;
	const int32 N = BenchSize;

	float bestD2 = TNumericLimits<float>::Max();
	int32 bestG  = INDEX_NONE;

	auto try_slot = [&](int32 local, const FPlayerTile& T)
	{
		const float d2 = (T.Position - World).SizeSquared2D();
		if (d2 < bestD2) { bestD2 = d2; bestG =  local; }
	};

	for (int32 i=0;i<N;++i)
		if (PlayerBench.IsValidIndex(i)) try_slot(i, PlayerBench[i]);


	if (bestG == INDEX_NONE) return false;

	const float DefSnap = (FieldTileWidthX > 0.f) ? (FieldTileWidthX * 0.6f) : 120.f;
	const float Snap    = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;
	if (bestD2 > Snap*Snap) return false;

	OutLocalBenchIndex = bestG;
	return true;
}

bool APCPlayerBoard::WorldAnyTile(const FVector& World, bool bPreferField, bool& bOutIsField, int32& OutY, int32& OutX,
	int32& OutLocalBenchIndex, FVector& OutSnapPos, float MaxSnapField, float MaxSnapBench, bool bRequireUnit) const
{
	 bOutIsField        = false;
    OutY = OutX        = INDEX_NONE;
    OutLocalBenchIndex = INDEX_NONE;
    OutSnapPos         = World;

    int32 Y = INDEX_NONE, X = INDEX_NONE, LocalBench = INDEX_NONE;

    const bool bFieldHit = WorldToField(World, Y, X, MaxSnapField);
    const bool bBenchHit = WorldToBench(World, LocalBench, MaxSnapBench);

    if (!bFieldHit && !bBenchHit)
        return false;

    // ── 필드 후보 평가
    bool bFieldCandidate = false;
    FVector PField = FVector::ZeroVector;
    if (bFieldHit)
    {
        const int32 Idx = IndexOf(Y, X);
        if (PlayerField.IsValidIndex(Idx))
        {
            const bool bHasUnit = (PlayerField[Idx].Unit != nullptr);
            if (!bRequireUnit || bHasUnit)
            {
                bFieldCandidate = true;
                PField = PlayerField[Idx].Position;
            }
        }
    }

    // ── 벤치 후보 평가 (로컬 인덱스만 사용)
    bool bBenchCandidate = false;
    FVector PBench = FVector::ZeroVector;
    if (bBenchHit && PlayerBench.IsValidIndex(LocalBench))
    {
        const bool bHasUnit = (PlayerBench[LocalBench].Unit != nullptr);
        if (!bRequireUnit || bHasUnit)
        {
            bBenchCandidate = true;
            PBench = PlayerBench[LocalBench].Position;
        }
    }

    if (!bFieldCandidate && !bBenchCandidate)
        return false;

    // ── 둘 다 후보면 더 가까운 쪽(동률 시 bPreferField에 살짝 가중치)
    if (bFieldCandidate && bBenchCandidate)
    {
        const float df2 = FVector::DistSquared2D(PField, World);
        const float db2 = FVector::DistSquared2D(PBench, World);
        constexpr float TieBias = 1.02f; // 근소 차이 시 필드 우대 옵션
        const bool ChooseField = bPreferField ? (df2 <= db2 * TieBias) : (df2 < db2);

        if (ChooseField)
        {
            bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PField;
        }
        else
        {
            bOutIsField = false; OutLocalBenchIndex = LocalBench; OutSnapPos = PBench;
        }
        return true;
    }

    // ── 하나만 후보
    if (bFieldCandidate)
    {
        bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PField;
        return true;
    }
    else
    {
        bOutIsField = false; OutLocalBenchIndex = LocalBench; OutSnapPos = PBench;
        return true;
    }
}

void APCPlayerBoard::AttachToCombatBoard(APCCombatBoard* CombatBoardToFollow, bool bIsGuest)
{
	if (!IsValid(CombatBoardToFollow))
		return;

	// 홈 위치 저장(한 번만 저장하고, 이미 붙어있다면 업데이트만)
	if (!BattleBoard.IsValid())
	{
		SaveHomeTransform = GetActorTransform();
	}

	BattleBoard = CombatBoardToFollow;

	// 전장 보드 기준으로 위치/회전 맞추기 (+180 Yaw)
	const FTransform BoardXf = CombatBoardToFollow->GetActorTransform();
	const FVector   NewLoc   = BoardXf.GetLocation() + AttachWorldOffset;
	const FRotator  BoardRot = BoardXf.Rotator();
	const FRotator  NewRot   = FRotator(
		BoardRot.Pitch,
		FMath::UnwindDegrees(BoardRot.Yaw + AttachYawOffsetDeg),  // 요구: 180도 반전
		BoardRot.Roll
	);
	
	// 월드 변환 먼저 적용한 뒤…
	SetActorLocationAndRotation(NewLoc, NewRot, /*bSweep=*/false, /*OutHit=*/nullptr, ETeleportType::TeleportPhysics);

	// 전장 보드에 부착(보드가 이동하면 같이 따라가도록) — 월드 변환 유지
	AttachToActor(CombatBoardToFollow, FAttachmentTransformRules::KeepWorldTransform);
}

void APCPlayerBoard::DetachFromCombatBoard()
{
	// 부착 해제
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 원래 홈 위치/회전으로 복귀
	if (!SaveHomeTransform.Equals(FTransform::Identity))
	{
		SetActorTransform(SaveHomeTransform, /*bSweep=*/false, /*OutHit=*/nullptr, ETeleportType::TeleportPhysics);
	}

	// 벤치 유닛들을 현재 PlayerBoard 기준 벤치 타일 위치로 재스냅
	ResnapBenchUnitsToBoard();

	BattleBoard = nullptr;
}

void APCPlayerBoard::ResnapBenchUnitsToBoard()
{
	for (int32 i = 0; i < PlayerBench.Num(); ++i)
	{
		if (APCBaseUnitCharacter* Unit = PlayerBench[i].Unit)
		{
			const FVector Loc = PlayerBench[i].Position;
			const FRotator Rot = FRotator(0,0,0);
			Unit->SetActorLocationAndRotation(Loc, Rot, false, nullptr, ETeleportType::TeleportPhysics);
			Unit->ChangedOnTile(false);
		}
	}
}

bool APCPlayerBoard::CopyFieldToTileManager(class UPCTileManager* TM, bool bMirrorRows, bool bMirrorCols)
{
	if (!TM || TM->Rows != Rows || TM->Cols != Cols) return false;

	// 내 필드에 있는 유닛만 TM.Field로 복제
	for (int32 y=0;y<Cols;++y)
		for (int32 x=0;x<Rows;++x)
		{
			const int32 i = IndexOf(y,x);
			if (!PlayerField.IsValidIndex(i)) continue;
			if (APCBaseUnitCharacter* U = PlayerField[i].Unit)
			{
				const int32 nRow = bMirrorRows ? (Rows-1-x) : x;
				const int32 nCol = bMirrorCols ? (Cols-1-y) : y;
				TM->PlaceUnitOnField(nCol, nRow, U, ETileFacing::Auto);
			}
		}
	return true;
}

bool APCPlayerBoard::CopyTileManagerToField(class UPCTileManager* TM, bool bMirrorRows, bool bMirrorCols)
{
	if (!TM || TM->Rows != Rows || TM->Cols != Cols) return false;

	// 일단 내 필드 비우고
	for (auto& T : PlayerField) T.Unit = nullptr;

	// TM 필드에서 유닛을 가져와서 내 필드에 기록
	for (int32 y=0;y<TM->Cols;++y)
		for (int32 x=0;x<TM->Rows;++x)
		{
			if (APCBaseUnitCharacter* U = TM->GetFieldUnit(y,x))
			{
				const int32 nRow = bMirrorRows ? (Rows-1-x) : x;
				const int32 nCol = bMirrorCols ? (Cols-1-y) : y;
				PlaceUnitOnField(nCol, nRow, U);
			}
		}
	return true;                                                                    
}

void APCPlayerBoard::SetHighlight(bool bOnField, bool bOnBench)
{
	if (PlayerFieldHISM)
		PlayerFieldHISM->SetOverlayMaterial(bOnField ? FieldTileOverlayMaterial : nullptr);
	if (PlayerBenchHISM)
		PlayerBenchHISM->SetOverlayMaterial(bOnBench ? BenchTileOverlayMaterial : nullptr);
}

void APCPlayerBoard::MakeSnapshot(FPlayerBoardSnapshot& Out) const
{
	Out.FieldUnits.SetNum(Rows*Cols);
	for (int32 i=0;i<Out.FieldUnits.Num();++i) Out.FieldUnits[i] = PlayerField.IsValidIndex(i) ? PlayerField[i].Unit : nullptr;
	Out.BenchUnits.SetNum(PlayerBench.Num());
	for (int32 i=0;i<Out.BenchUnits.Num();++i) Out.BenchUnits[i] = PlayerBench[i].Unit;
}

void APCPlayerBoard::ApplySnapshot(const FPlayerBoardSnapshot& In)
{
	const int32 NField = Rows*Cols;
	for (int32 i=0;i<NField && i<In.FieldUnits.Num(); ++i)
		if (PlayerField.IsValidIndex(i)) PlayerField[i].Unit = In.FieldUnits[i];
	for (int32 i=0;i<PlayerBench.Num() && i<In.BenchUnits.Num(); ++i)
		PlayerBench[i].Unit = In.BenchUnits[i];
}

FVector APCPlayerBoard::GetFieldWorldPos(int32 Y, int32 X) const
{
	const int32 i = IndexOf(Y,X);
	return PlayerField.IsValidIndex(i) ? PlayerField[i].Position : FVector::ZeroVector;
}

FVector APCPlayerBoard::GetBenchWorldPos(int32 LocalBenchIndex) const
{
	return PlayerBench.IsValidIndex(LocalBenchIndex) ? PlayerBench[LocalBenchIndex].Position : FVector::ZeroVector;
}



