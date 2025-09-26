// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "Windows/WindowsApplication.h"


// Sets default values for this component's properties
UPCTileManager::UPCTileManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
}

APCCombatBoard* UPCTileManager::GetCombatBoard() const
{
	return CachedCombatBoard ? CachedCombatBoard.Get() : Cast<APCCombatBoard>(GetOwner());
}

int32 UPCTileManager::GetBoardIndex()
{
	if (APCCombatBoard* MyBoard = GetCombatBoard())
	{
		return MyBoard->BoardSeatIndex;
	}
	return INDEX_NONE;
}

bool UPCTileManager::IsInRange(int32 Y, int32 X) const
{
	return (Y >= 0 && Y < Cols && X >= 0 && X < Rows);
}

void UPCTileManager::QuickSetUp()
{
	CachedCombatBoard = Cast<APCCombatBoard>(GetOwner());
	CreateField();
	CreateBench();
}

FRotator UPCTileManager::CalcUnitRotation(APCBaseUnitCharacter* Unit, ETileFacing Facing) const
{
	const APCCombatBoard* Board = GetCombatBoard();
	float Yaw = Board ? Board->GetActorRotation().Yaw : 0;
	Yaw += FacingYawOffsetDeg;

	bool bEnemy = false;
	switch (Facing)
	{
	case ETileFacing::Friendly : bEnemy = false;
		break;
	case ETileFacing::Enemy : bEnemy = true;
		break;
	case ETileFacing::Auto:
		default:
		bEnemy = (Board && Unit) ? (Unit->GetTeamIndex() != Board->BoardSeatIndex) : false;
		break;
	}
	if (bEnemy) Yaw += 180.f;
	return FRotator(0.f, FMath::UnwindDegrees(Yaw),0.f);
}

bool UPCTileManager::PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit, ETileFacing FacingOverride)
{
	const int32 i = Y * Rows + X;
	if (!Field.IsValidIndex(i) || !Unit || !Field[i].IsEmpty())
		return false;

	EnsureExclusive(Unit);

	Field[i].Unit = Unit;
	APCCombatBoard* Board = GetCombatBoard();
	const FVector Loc = Field[i].Position;
	const FRotator Rot = CalcUnitRotation(Unit, FacingOverride);

	Unit->SetOnCombatBoard(Board);
	Unit->SetActorLocationAndRotation(Loc, Rot, false, nullptr, ETeleportType::TeleportPhysics);
	Unit->ChangedOnTile(true);
	BindToUnit(Unit);

	return true;
}

bool UPCTileManager::PlaceUnitOnBench(int32 BenchIndex, APCBaseUnitCharacter* Unit, ETileFacing FacingOverride)
{
	bool bEnemy;
	int32 Local;
	if (!SplitGlobalBenchIndex(BenchIndex, bEnemy, Local)) return false;
	TArray<FTile>& A = bEnemy ? EnemyBench : Bench;
	if (!A.IsValidIndex(Local) || !Unit || !A[Local].IsEmpty()) return false;

	EnsureExclusive(Unit);
	A[Local].Unit = Unit;

	APCCombatBoard* Board = GetCombatBoard();
	const FVector  Loc = A[Local].Position;
	const FRotator Rot = CalcUnitRotation(Unit, FacingOverride);

	Unit->SetOnCombatBoard(Board);
	Unit->SetActorLocationAndRotation(Loc, Rot, false, nullptr, ETeleportType::ResetPhysics);
	Unit->ChangedOnTile(false);
	return true;
}

bool UPCTileManager::RemoveFromField(int32 Y, int32 X, bool bPreserveUnitBoard)
{
	const int32 i = Y * Rows + X;
	if (!Field.IsValidIndex(i))
		return false;

	if (APCBaseUnitCharacter* Unit = Field[i].Unit)
	{
		UnbindFromUnit(Unit);
		if (!bPreserveUnitBoard)
		{
			Field[i].Unit->SetOnCombatBoard(nullptr);
		}
	}
		
	Field[i].Unit = nullptr;
	return true;
}

APCBaseUnitCharacter* UPCTileManager::GetFieldUnit(int32 Y, int32 X) const
{
	const int32 i = Y * Rows + X;
	return Field.IsValidIndex(i) ? Field[i].Unit : nullptr;
}

FVector UPCTileManager::GetFieldUnitLocation(APCBaseUnitCharacter* InUnit) const
{
	if (!InUnit)
		return FVector::ZeroVector;

	for (int32 x = 0; x < Rows; ++x)
	{
		for (int32 y = 0; y < Cols; ++y)
		{
			const int32 i = y * Rows + x;
			if (Field.IsValidIndex(i) && Field[i].Unit == InUnit)
			{
				return Field[i].Position;
			}
		}
	}
	return FVector::ZeroVector;
}

FIntPoint UPCTileManager::GetFieldUnitGridPoint(APCBaseUnitCharacter* InUnit) const
{
	if (!InUnit)
		return FIntPoint::NoneValue;
	for (int32 x = 0; x < Rows; ++x)
	{
		for (int32 y = 0; y < Cols; ++y)
		{
			const int32 i = y * Rows + x;
			if (Field.IsValidIndex(i) && Field[i].Unit == InUnit)
			{
				return Field[i].UnitIntPoint;
			}
		}
	}
	
	return FIntPoint::NoneValue;
	
}

FVector UPCTileManager::GetTileWorldPosition(int32 Y, int32 X) const
{
	const int32 i = Y * Rows + X;
	return Field.IsValidIndex(i) ? Field[i].Position : FVector::ZeroVector;
}

FVector UPCTileManager::GetTileLocalPosition(int32 Y, int32 X) const
{
	if (!GetOwner())
		return FVector::ZeroVector;
	const FVector WorldPosition = GetTileWorldPosition(Y, X);
	return GetOwner()->GetTransform().InverseTransformPosition(WorldPosition);
}


bool UPCTileManager::RemoveFromBench(int32 BenchIndex, bool bPreserveUnitBoard)
{
	bool bEnemy;
	int32 Local;
	if (!SplitGlobalBenchIndex(BenchIndex, bEnemy, Local)) return false;
	TArray<FTile>& A = bEnemy ? EnemyBench : Bench;
	if (!A.IsValidIndex(Local)) return false;

	if (!bPreserveUnitBoard && A[Local].Unit)
		A[Local].Unit->SetOnCombatBoard(nullptr);

	A[Local].Unit = nullptr;
	return true;
}

APCBaseUnitCharacter* UPCTileManager::GetBenchUnit(int32 BenchIndex) const
{
	bool bEnemy;
	int32 Local;
	if (!SplitGlobalBenchIndex(BenchIndex, bEnemy, Local)) return nullptr;
	const TArray<FTile>& A = bEnemy ? EnemyBench : Bench;
	return A.IsValidIndex(Local) ? A[Local].Unit : nullptr;
}

FVector UPCTileManager::GetBenchWorldPosition(int32 BenchIndex) const
{
	bool bEnemy; int32 Local;
	if (!SplitGlobalBenchIndex(BenchIndex, bEnemy, Local)) return FVector::ZeroVector;
	const TArray<FTile>& A = bEnemy ? EnemyBench : Bench;
	return A.IsValidIndex(Local) ? A[Local].Position : FVector::ZeroVector;
}

FVector UPCTileManager::GetBenchLocalPosition(int32 BenchIndex) const
{
	if (!GetOwner())
		return FVector::ZeroVector;
	const FVector WorldPosition = GetBenchWorldPosition(BenchIndex);
	return GetOwner()->GetTransform().InverseTransformPosition(WorldPosition);
}

int32 UPCTileManager::GetBenchUnitIndex(APCBaseUnitCharacter* Unit) const
{
	if (!Unit) return INDEX_NONE;

	const int32 N = BenchSlotsPerSide;

	// Friendly 줄
	for (int32 i = 0; i < N; ++i)
		if (Bench.IsValidIndex(i) && Bench[i].Unit == Unit)
			return MakeGlobalBenchIndex(/*bEnemySide=*/false, i);

	// Enemy 줄
	for (int32 i = 0; i < N; ++i)
		if (EnemyBench.IsValidIndex(i) && EnemyBench[i].Unit == Unit)
			return MakeGlobalBenchIndex(/*bEnemySide=*/true, i);

	return INDEX_NONE;
}

void UPCTileManager::ClearAll()
{
	for (auto& FieldTile : Field)
	{
		FieldTile.Unit = nullptr;
	}
	for (auto& BenchTile : Bench)
	{
		BenchTile.Unit = nullptr;
	}

	for (auto& EnemyBenchTile : EnemyBench)
	{
		EnemyBenchTile.Unit = nullptr;
	}
}

int32 UPCTileManager::GetBenchIndex(bool bEnemySide, int32 LocalIndex) const
{
	const int32 N = BenchSlotsPerSide;
	if (LocalIndex < 0 || LocalIndex >= N)
		return INDEX_NONE;
	return (bEnemySide ? N : 0) + LocalIndex;
}

bool UPCTileManager::RemoveFromBoard(APCBaseUnitCharacter* Unit)
{
	auto FieldGridPoint = GetFieldUnitGridPoint(Unit);
	auto BenchIndex = GetBenchUnitIndex(Unit);
			
	if (FieldGridPoint != FIntPoint::NoneValue)
	{
		RemoveFromField(FieldGridPoint.Y, FieldGridPoint.X, false);
		return true;
	}
	else if (BenchIndex != INDEX_NONE)
	{
		RemoveFromBench(BenchIndex, false);
		return true;
	}

	return false;
}

bool UPCTileManager::IsTileFree(int32 Y, int32 X) const
{
	int32 Index;
	return IsValidTile(Y,X,Index) ? Field[Index].IsFree() : false;
}

bool UPCTileManager::CanUse(int32 Y, int32 X, const APCBaseUnitCharacter* InUnit) const
{
	int32 Index;
	return IsValidTile(Y,X, Index) ? Field[Index].CanBeUsedBy(InUnit) : false; 
}

// 어딘가에 예약이 되어있는지
bool UPCTileManager::HasAnyReservation(const APCBaseUnitCharacter* InUnit) const
{
	if (!InUnit) return false;
	for (const FTile& Tile : Field)
	{
		if (Tile.IsReservedBy(InUnit))
		{
			return true;
		}
	}
	return false;
}

bool UPCTileManager::SetTileState(int32 Y, int32 X, APCBaseUnitCharacter* InUnit, ETileAction Action)
{
	if (!InUnit) return false;

	int32 Index;
	if (!IsValidTile(Y,X,Index)) return false;

	FTile& Tile = Field[Index];

	switch (Action)
	{
	case ETileAction::Reserve:
		// 다른 유닛이 점유/ 예약 중이면 실패, 내 점유/내 예약 빈칸만 허용
		if (!Tile.CanBeUsedBy(InUnit))
		{
			return false;
		}
		Tile.ReservedUnit = InUnit;
		return true;

	case ETileAction::Occupy:
		// 남이 점유 중이거나 남이 예약 중이면 실패
		if (!Tile.IsOwnedBy(InUnit) && Tile.Unit != nullptr || Tile.IsReserved() && !Tile.IsReservedBy(InUnit))
		{
			return false;
		}
		Tile.Unit = InUnit;
		Tile.ReservedUnit = nullptr;
		InUnit->SetOnCombatBoard(CachedCombatBoard.Get());
		//InUnit->SetActorLocation(Tile.Position);
		return true;

	case ETileAction::Release:
		{
			bool bAny = false;
			if (Tile.IsOwnedBy(InUnit))
			{
				Tile.Unit = nullptr;
				bAny = true;
			}
			if (Tile.IsReservedBy(InUnit))
			{
				Tile.ReservedUnit = nullptr;
				bAny = true;
			}
		}
	}
	return false;
}

void UPCTileManager::ClearAllForUnit(APCBaseUnitCharacter* InUnit)
{
	if (!InUnit) return;

	for (FTile& Tile : Field)
	{
		if (Tile.IsOwnedBy(InUnit))
		{
			Tile.Unit = nullptr;
		}
		if (Tile.IsReservedBy(InUnit))
		{
			Tile.ReservedUnit = nullptr;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ClearAllForUnit"));

	InUnit->SetOnCombatBoard(nullptr);
	UnbindFromUnit(InUnit);
}

// 타일 유효성 헬퍼
bool UPCTileManager::IsValidTile(int32 Y, int32 X, int32& OutIndex) const
{
	if (!IsInRange(Y, X))
		return false;
	OutIndex = IndexOf(Y,X);
	return Field.IsValidIndex(OutIndex);
	
}

int32 UPCTileManager::MirrorBenchIndex(int32 Index) const
{
	bool bEnemy;
	int32 Local;
	if (!SplitGlobalBenchIndex(Index, bEnemy,Local))
	{
		return INDEX_NONE;
	}
	return MakeGlobalBenchIndex(!bEnemy, Local);
}

int32 UPCTileManager::MakeGlobalBenchIndex(bool bEnemySide, int32 LocalIndex) const
{
	const int32 N = BenchSlotsPerSide;
	if (LocalIndex < 0 || LocalIndex >= N) return INDEX_NONE;
	return bEnemySide ? (N + LocalIndex) : LocalIndex;
}

bool UPCTileManager::SplitGlobalBenchIndex(int32 GlobalIndex, bool& bEnemySide, int32& Local) const
{
	const int32 N = BenchSlotsPerSide;
	if (GlobalIndex < 0 || GlobalIndex >= 2*N) return false;
	bEnemySide = (GlobalIndex >= N);
	Local = bEnemySide ? (GlobalIndex - N) : GlobalIndex;
	return true;
}


void UPCTileManager::MoveUnitsMirroredTo(UPCTileManager* TargetField, bool bMirrorRows, bool bMirrorCols,
                                         bool bIncludeBench)
{
	 if (!TargetField || Rows != TargetField->Rows || Cols != TargetField->Cols)
        return;

    // --- 필드 캡처
    struct FCapturedField { int32 Col; int32 Row; TWeakObjectPtr<APCBaseUnitCharacter> Unit; };
    TArray<FCapturedField> CF; CF.Reserve(Field.Num());

    for (int32 r=0;r<Rows;++r)
        for (int32 c=0;c<Cols;++c)
            if (APCBaseUnitCharacter* U = GetFieldUnit(c, r))
                CF.Add({c, r, U});

    // --- 벤치 캡처 (글로벌 인덱스)
    struct FCapturedBench { int32 GlobalIndex; TWeakObjectPtr<APCBaseUnitCharacter> Unit; };
    TArray<FCapturedBench> CB;

    const int32 NThis   = BenchSlotsPerSide;
    const int32 NTarget = TargetField->BenchSlotsPerSide;

    const bool bCanMirrorBench =
        (NThis > 0) && (NTarget == NThis) &&
        (Bench.Num() == NThis) && (EnemyBench.Num() == NThis) &&
        (TargetField->Bench.Num() == NTarget) && (TargetField->EnemyBench.Num() == NTarget);

    if (bIncludeBench)
    {
        // 0..2N-1 글로벌 검색
        for (int32 g=0; g<2*NThis; ++g)
            if (APCBaseUnitCharacter* U = GetBenchUnit(g))
                CB.Add({g, U});
    }

    // --- 필드 이동
    for (const auto& E : CF)
    {
        const int32 nRow = bMirrorRows ? (Rows - 1 - E.Row) : E.Row;
        const int32 nCol = bMirrorCols ? (Cols - 1 - E.Col) : E.Col;

        if (TargetField->PlaceUnitOnField(nCol, nRow, E.Unit.Get(), ETileFacing::Enemy))
            RemoveFromField(E.Col, E.Row, /*preserve*/true);
    }

    // --- 벤치 이동
    if (bIncludeBench)
    {
        for (const auto& E : CB)
        {
            int32 NewGlobal = E.GlobalIndex;

            if (bMirrorCols)
            {
                if (bCanMirrorBench)
                {
                    // 사이드만 토글
                    NewGlobal = TargetField->MirrorBenchIndex(E.GlobalIndex);
                }
                else
                {
                    // 폴백: 전체 2N 범위 안에서 좌우 반전
                    const int32 TwoN = 2 * FMath::Max(1, NTarget);
                    NewGlobal = (TwoN - 1) - (E.GlobalIndex % TwoN);
                }
            }

            if (TargetField->PlaceUnitOnBench(NewGlobal, E.Unit.Get(), ETileFacing::Enemy))
                RemoveFromBench(E.GlobalIndex, /*preserve*/true);
        }
    }
}

bool UPCTileManager::EnsureExclusive(APCBaseUnitCharacter* InUnit)
{
	if (!InUnit) return false;

	const FIntPoint GridPoint = GetFieldUnitGridPoint(InUnit);
	if (GridPoint != FIntPoint::NoneValue)
	{
		RemoveFromField(GridPoint.Y, GridPoint.X, false);
		return true;
	}

	const int32 BenchIdx = GetBenchUnitIndex(InUnit);
	if (BenchIdx != INDEX_NONE)
	{
		RemoveFromBench(BenchIdx, false);
		return true;
	}
	return false;
}

void UPCTileManager::CreateField()
{
	Field.SetNum(Rows * Cols);

	const FVector OwnerBoardLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	const float Radios = TileWidthX * 0.5;
	const float Xs = 1.5f * Radios;
	const float Ys = FMath::Sqrt(3.f) * Radios;

	const FVector Base = OwnerBoardLocation + FVector(FirstFieldLocal.X, FirstFieldLocal.Y, 0);

	for (int32 r = 0; r < Rows; ++r)
	{
		const int32 rLocal = bRowZeroAtBottom ? r : (Rows - 1 - r);

		for (int32 c = 0; c < Cols; ++c)
		{
			const int32 i = c * Rows + r;
			const float x = c * Xs;

			const float RowShift = ((c&1) ? OddColumRowShift : 0.0f);
			const float y = (rLocal + RowShift) * Ys;
			Field[i].UnitIntPoint = FIntPoint(r,c);
			Field[i].Position = Base + FVector(x, y, 0);
			Field[i].bIsField = true;
			Field[i].Unit = nullptr;
		}
	}
	
}

void UPCTileManager::CreateBench()
{
	const int32 N = FMath::Max(0, BenchSlotsPerSide);
	Bench.SetNum(N);
	EnemyBench.SetNum(N);

	const FVector OwnerLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	const FVector FirstPlayerLoc = OwnerLoc + FVector(BenchTilePlayerLocal.X, BenchTilePlayerLocal.Y, 0);
	const FVector FirstEnemyLoc = OwnerLoc + FVector(BenchTileEnemyLocal.X, BenchTileEnemyLocal.Y, 0);

	for ( int32 i = 0; i < N; ++i)
	{
		Bench[i].Position = FirstPlayerLoc + FVector(0.f, i * BenchStepLocalY, 0.f);
		Bench[i].bIsField = false;
		Bench[i].Unit = nullptr;

		EnemyBench[i].Position = FirstEnemyLoc  + FVector(0.f, i * -BenchStepLocalY, 0.f);
		EnemyBench[i].bIsField = false;
		EnemyBench[i].Unit = nullptr;
	}
}

void UPCTileManager::BeginPlay()
{
	Super::BeginPlay();
	CachedCombatBoard = Cast<APCCombatBoard>(GetOwner());
}

void UPCTileManager::DebugLogField(bool bAsGrid /*=true*/, bool bShowOccupiedList /*=true*/, const FString& Tag) const
{
	const ENetMode NM = GetNetMode();
	UE_LOG(LogTemp, Warning, TEXT("[%s] World=%s NetMode=%d HasAuth=%d Owner=%s"),
		*Tag, *GetWorld()->GetName(), (int)NM,
		GetOwner() ? (int)GetOwner()->HasAuthority() : -1,
		*GetOwner()->GetName());
	
	const int32 NExpected = Rows * Cols;
    UE_LOG(LogTemp, Log, TEXT("=== [TileManager] Field Debug === Rows=%d Cols=%d Field.Num=%d (Expect=%d)"),
        Rows, Cols, Field.Num(), NExpected);

    // 그리드 출력 (행=Y, 열=X)
    if (bAsGrid)
    {
        for (int32 y = 0; y < Rows; ++y)
        {
            FString Line;
            Line.Reserve(Cols * 4);
            for (int32 x = 0; x < Cols; ++x)
            {
                const int32 i = y * Rows + x; // ⚠️ 네가 쓰는 규칙: IndexOf(Y,X) = Y*Rows + X

                TCHAR Mark = TEXT('·');     // 빈칸
                if (!Field.IsValidIndex(i))
                {
                    Mark = TEXT('X');       // 인덱스 이상
                }
                else if (Field[i].Unit != nullptr)
                {
                    Mark = TEXT('O');       // 유닛 존재
                }

                // 보기 좋게 (예: "O  " / "·  " / "X  ")
                Line.AppendChar(Mark);
                Line.Append(TEXT("  "));
            }
            UE_LOG(LogTemp, Log, TEXT("[Y=%d] %s"), y, *Line);
        }
    }

    // 점유 목록 상세
    if (bShowOccupiedList)
    {
        int32 Occupied = 0, Invalid = 0;
        for (int32 i = 0; i < Field.Num(); ++i)
        {
            const bool bValid = Field.IsValidIndex(i);
            if (!bValid) { ++Invalid; continue; }

            // 역변환: i = Y*Rows + X  =>  Y = i / Rows,  X = i % Rows
            const int32 y = i / Rows;
            const int32 x = i % Rows;

            if (Field[i].Unit)
            {
                ++Occupied;
                UE_LOG(LogTemp, Log, TEXT("[#%d] (Y=%d, X=%d) Unit=%s"),
                    i, y, x, *Field[i].Unit->GetName());
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Occupied=%d, InvalidIndices=%d"), Occupied, Invalid);

        // 배열 크기 검증도 한 번
        if (Field.Num() != NExpected)
        {
            UE_LOG(LogTemp, Warning, TEXT("[WARN] Field.Num(%d) != Rows*Cols(%d). 인덱싱 오동작 가능."),
                Field.Num(), NExpected);
        }
    }
	

    UE_LOG(LogTemp, Log, TEXT("=== [TileManager] End ==="));
}

void UPCTileManager::DebugExplainTile(int32 Y, int32 X, const FString& Tag) const
{
	int32 Index = -1;
	const bool bInRange = IsInRange(Y,X);
	const bool bValid   = bInRange && IsValidTile(Y,X,Index);
	UE_LOG(LogTemp, Warning, TEXT("[Explain %s] Y=%d X=%d InRange=%d Valid=%d Index=%d"),
		*Tag, Y, X, bInRange, bValid, Index);

	if (!bValid) return;

	const FTile& T = Field[Index];
	UE_LOG(LogTemp, Warning, TEXT("  Unit=%s  Reserved=%d  IsFree=%d  CanUse(self)=%d"),
		T.Unit ? *T.Unit->GetName() : TEXT("null"),
		(int32)T.IsReserved(),
		(int32)T.IsFree(),
		(int32)T.CanBeUsedBy(nullptr)); // 원하면 OwnerUnit 넘겨서 체크
}

// Drag&Drop Helper

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

bool UPCTileManager::WorldToField(const FVector& WorldLoc, int32& OutY, int32& OutX, float MaxSnapDist) const
{
	OutY = INDEX_NONE;
	OutX = INDEX_NONE;

	if (Field.Num() <= 0 || Rows <= 0 || Cols <= 0)
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
			if (!Field.IsValidIndex(i)) continue;

			const float d2 = Dist2_2D(Field[i].Position, WorldLoc);
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
	const float DefSnap = (TileWidthX > 0.f) ? (TileWidthX * 0.6f) : 120.f;
	const float Snap = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;

	if (BestD2 > (Snap * Snap))
		return false;

	OutY = BestY;
	OutX = BestX;
	
	return true;
	
	
}

bool UPCTileManager::WorldToBench(const FVector& World, int32& OutBenchIndex, float MaxSnapDist) const
{
	OutBenchIndex = INDEX_NONE;
	const int32 N = BenchSlotsPerSide;

	float bestD2 = TNumericLimits<float>::Max();
	int32 bestG  = INDEX_NONE;

	auto try_slot = [&](bool bEnemySide, int32 local, const FTile& T)
	{
		const float d2 = (T.Position - World).SizeSquared2D();
		if (d2 < bestD2) { bestD2 = d2; bestG = MakeGlobalBenchIndex(bEnemySide, local); }
	};

	for (int32 i=0;i<N;++i)
		if (Bench.IsValidIndex(i)) try_slot(false, i, Bench[i]);
	for (int32 i=0;i<N;++i)
		if (EnemyBench.IsValidIndex(i))    try_slot(true,  i, EnemyBench[i]);

	if (bestG == INDEX_NONE) return false;

	const float DefSnap = (TileWidthX > 0.f) ? (TileWidthX * 0.6f) : 120.f;
	const float Snap    = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;
	if (bestD2 > Snap*Snap) return false;

	OutBenchIndex = bestG;
	return true;
}

bool UPCTileManager::WorldAnyTile(const FVector& World, bool bPreferField,
    bool& bOutIsField, int32& OutY, int32& OutX, int32& OutBenchIndex, FVector& OutSnapPos,
    float MaxSnapDistField, float MaxSnapDistBench,
    bool bRequireUnit) const
{
    bOutIsField   = false;
    OutY = OutX   = INDEX_NONE;
    OutBenchIndex = INDEX_NONE;
    OutSnapPos    = World;

    int32 Y, X, B;
    const bool bFieldHit = WorldToField(World, Y, X, MaxSnapDistField);
    const bool bBenchHit = WorldToBench(World, B, MaxSnapDistBench);

    if (!bFieldHit && !bBenchHit) return false;

    // 필드 후보
    bool bFieldCandidate = false; FVector PField(0);
    if (bFieldHit)
    {
        const int32 Idx = IndexOf(Y, X);
        if (Field.IsValidIndex(Idx))
        {
            const bool bHasUnit = (Field[Idx].Unit != nullptr);
            if (!bRequireUnit || bHasUnit)
            { bFieldCandidate = true; PField = Field[Idx].Position; }
        }
    }

    // 벤치 후보 (글로벌 → 사이드/로컬 쪼개서 올바른 배열 사용)
    bool bBenchCandidate = false; FVector PBench(0);
    if (bBenchHit)
    {
        bool bEnemy=false; int32 Local=-1;
        if (SplitGlobalBenchIndex(B, bEnemy, Local))
        {
            const TArray<FTile>& A = bEnemy ? EnemyBench : Bench;
            if (A.IsValidIndex(Local))
            {
                const bool bHasUnit = (A[Local].Unit != nullptr);
                if (!bRequireUnit || bHasUnit)
                { bBenchCandidate = true; PBench = A[Local].Position; }
            }
        }
    }

    if (!bFieldCandidate && !bBenchCandidate)
        return false;

    if (bFieldCandidate && bBenchCandidate)
    {
        const float df2 = FVector::DistSquared2D(PField, World);
        const float db2 = FVector::DistSquared2D(PBench, World);
        constexpr float TieBias = 1.02f;
        const bool ChooseField = bPreferField ? (df2 <= db2 * TieBias) : (df2 < db2);

        if (ChooseField)
        {
	        bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PField;
        }
        else
        {
	        bOutIsField = false; OutBenchIndex = B; OutSnapPos = PBench;
        }
        return true;
    }

    if (bFieldCandidate)
    {
    	bOutIsField = true;  OutY = Y; OutX = X; OutSnapPos = PField;
    	return true;
    }
    else
    {
	    bOutIsField = false; OutBenchIndex = B; OutSnapPos = PBench;
    	return true;
    }
}

TArray<APCBaseUnitCharacter*> UPCTileManager::GetAllUnitByTag(FGameplayTag UnitTag, int32 TeamSeat)
{
	TArray<APCBaseUnitCharacter*> Out;
	if (!UnitTag.IsValid()) return Out;

	auto AddIf = [&](APCBaseUnitCharacter* U){
		if (IsValid(U) && U->GetUnitTag().IsValid() && U->GetUnitTag().MatchesTag(UnitTag) && U->GetTeamIndex() == TeamSeat)
		{
			Out.AddUnique(U);
		}
	};

	// 필드
	for (const auto& T : Field)
		AddIf(T.Unit);
	// 벤치 양쪽
	for (const auto& T : Bench)
		AddIf(T.Unit);
	return Out;
}

TArray<APCBaseUnitCharacter*> UPCTileManager::GetFieldUnitByTag(FGameplayTag UnitTag)
{
	TArray<APCBaseUnitCharacter*> AllUnits;
	if (!UnitTag.IsValid())
		return AllUnits;

	auto AddIfMatch = [&](APCBaseUnitCharacter* Unit)
	{
		if (!IsValid(Unit)) return;

		if (Unit->GetUnitTag().IsValid() && Unit->GetUnitTag().MatchesTag(UnitTag) && Unit->GetTeamIndex() == GetBoardIndex())
		{
			if (!AllUnits.Contains(Unit))
			{
				AllUnits.Add(Unit);
			}
		}
	};
	
	for (const FTile& FieldTile : Field)
	{
		AddIfMatch(FieldTile.Unit);
	}

	return AllUnits;
}

TArray<APCBaseUnitCharacter*> UPCTileManager::GetBenchUnitByTag(FGameplayTag UnitTag, int32 TeamSeat)
{
	TArray<APCBaseUnitCharacter*> Out;
	if (!UnitTag.IsValid()) return Out;

	auto AddIf = [&](APCBaseUnitCharacter* U){
		if (IsValid(U) && U->GetUnitTag().IsValid() &&
			U->GetUnitTag().MatchesTag(UnitTag) &&
			U->GetTeamIndex() == TeamSeat)
		{
			Out.AddUnique(U);
		}
	};

	for (const auto& T : Bench)      AddIf(T.Unit);
	for (const auto& T : EnemyBench) AddIf(T.Unit);
	return Out;
}

void UPCTileManager::BindToUnit(APCBaseUnitCharacter* Unit)
{
	if (!Unit) return;
	if (DeathBoundUnits.Contains(Unit)) return;

	Unit->OnUnitDied.AddDynamic(this, &UPCTileManager::OnBoundUnitDied);
	DeathBoundUnits.Add(Unit);
}

void UPCTileManager::UnbindFromUnit(APCBaseUnitCharacter* Unit)
{
	if (!Unit) return;
	if (!DeathBoundUnits.Contains(Unit)) return;

	Unit->OnUnitDied.RemoveDynamic(this, &UPCTileManager::UPCTileManager::OnBoundUnitDied);
	DeathBoundUnits.Remove(Unit);
}

void UPCTileManager::OnBoundUnitDied(APCBaseUnitCharacter* Unit)
{
	if (AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
			return;
	}

	ClearAllForUnit(Unit);
}

