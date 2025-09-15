// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"



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

bool UPCTileManager::PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit)
{
	const int32 i = Y * Rows + X;
	if (!Field.IsValidIndex(i) || !Unit || !Field[i].IsEmpty())
		return false;
	Field[i].Unit = Unit;

	APCCombatBoard* CombatBoard = GetCombatBoard();
	const FVector Loc = Field[i].Position;
	FRotator Rot = CombatBoard ? CombatBoard->GetActorRotation() : FRotator::ZeroRotator;
	
	if (CombatBoard && Unit->GetTeamIndex() == CachedCombatBoard->BoardSeatIndex)
	{
		Unit->SetOnCombatBoard(CombatBoard);
		Unit->SetActorLocation(Field[i].Position);
		Unit->ChangedOnTile(true);
	}
	else
	{
		Rot.Yaw = FMath::UnwindDegrees(Rot.Yaw + 180.f);
		Unit->SetOnCombatBoard(CombatBoard);
		Unit->SetActorLocationAndRotation(Loc,Rot,false,nullptr,ETeleportType::TeleportPhysics);
		Unit->ChangedOnTile(true);
	}
	return true;
}

bool UPCTileManager::RemoveFromField(int32 Y, int32 X, bool bPreserveUnitBoard)
{
	const int32 i = Y * Rows + X;
	if (!Field.IsValidIndex(i))
		return false;

	if (!bPreserveUnitBoard)
	{
		Field[i].Unit->SetOnCombatBoard(nullptr);
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

bool UPCTileManager::PlaceUnitOnBench(int32 BenchIndex, APCBaseUnitCharacter* Unit)
{
	if (!Bench.IsValidIndex(BenchIndex) || !Unit || !Bench[BenchIndex].IsEmpty())
		return false;

	Bench[BenchIndex].Unit = Unit;

	APCCombatBoard* CombatBoard = GetCombatBoard();
	const FVector Loc = Bench[BenchIndex].Position;
	FRotator Rot = CombatBoard ? CombatBoard->GetActorRotation() : FRotator::ZeroRotator;
	
	if (CombatBoard && Unit->GetTeamIndex() == CachedCombatBoard->BoardSeatIndex)
	{
		Unit->SetOnCombatBoard(CombatBoard);
		Unit->SetActorLocation(Loc);
	}
	else
	{
		Rot.Yaw = FMath::UnwindDegrees(Rot.Yaw + 180.f);
		Unit->SetOnCombatBoard(CombatBoard);
		Unit->SetActorLocationAndRotation(Loc,Rot,false,nullptr,ETeleportType::TeleportPhysics);
	}
	
	OnBenchUpdated.Broadcast();
	return true;
}

bool UPCTileManager::RemoveFromBench(int32 BenchIndex, bool bPreserveUnitBoard)
{
	if (!Bench.IsValidIndex(BenchIndex))
		return false;

	if (!bPreserveUnitBoard)
	{
		Bench[BenchIndex].Unit->SetOnCombatBoard(nullptr);
	}
	Bench[BenchIndex].Unit = nullptr;

	OnBenchUpdated.Broadcast();
	
	return true;
}

APCBaseUnitCharacter* UPCTileManager::GetBenchUnit(int32 BenchIndex) const
{
	return Bench.IsValidIndex(BenchIndex) ? Bench[BenchIndex].Unit : nullptr;
}

FVector UPCTileManager::GetBenchWorldPosition(int32 BenchIndex) const
{
	return Bench.IsValidIndex(BenchIndex) ? Bench[BenchIndex].Position : FVector::ZeroVector;
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
	if (!Unit)
		return INDEX_NONE;
	for (int32 i = 0; i < BenchSize; ++i)
	{
		if (Bench[i].Unit == Unit)
		{
			return i;
		}
	}
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
}

int32 UPCTileManager::GetBenchIndex(bool bEnemySide, int32 LocalIndex) const
{
	const int32 N = BenchSlotsPerSide;
	if (LocalIndex < 0 || LocalIndex >= N)
		return INDEX_NONE;
	return (bEnemySide ? N : 0) + LocalIndex;
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
		InUnit->SetActorLocation(Tile.Position);
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
	const int32 N = BenchSlotsPerSide;
	if (Index < 0 || Index >= 2*N)
		return INDEX_NONE;
	return(Index<N ? (Index+N) : (Index - N));
}



void UPCTileManager::MoveUnitsMirroredTo(UPCTileManager* TargetField, bool bMirrorRows, bool bMirrorCols,
	bool bIncludeBench)
{
	 if (!TargetField || Rows != TargetField->Rows || Cols != TargetField->Cols)
        return;

    // --- 필드 캡쳐: (Col,Row) 순서로 보관 ---
    struct FCapturedField
    {
        int32 Col;
        int32 Row;
        TWeakObjectPtr<APCBaseUnitCharacter> Unit;
    };

    TArray<FCapturedField> CapturedField;
    CapturedField.Reserve(Field.Num());

	for (int32 row = 0; row < Rows; ++row)
	{
		for (int32 col = 0; col < Cols; ++col)
		{
			{
				if (APCBaseUnitCharacter* U = GetFieldUnit(col, row)) 
				{
					CapturedField.Add({col, row, U});
				}
			}
		}
	}
	
   

    // --- 벤치 캡쳐 ---
    struct FCapturedBench { int32 Index; TWeakObjectPtr<APCBaseUnitCharacter> Unit; };
    TArray<FCapturedBench> CapturedBench;

    const int32 NThis   = BenchSlotsPerSide;
    const int32 NTarget = TargetField->BenchSlotsPerSide;
    const int32 TSize   = TargetField->BenchSize;

    const bool bDualBenchThis   = (Bench.Num()            == 2*NThis   && NThis   > 0);
    const bool bDualBenchTarget = (TargetField->Bench.Num() == 2*NTarget && NTarget > 0);

    if (bIncludeBench)
    {
        for (int32 i = 0; i < Bench.Num(); ++i)
            if (APCBaseUnitCharacter* U = GetBenchUnit(i))
                CapturedBench.Add({i, U});
    }

    // --- 필드 이동 ---
    for (const auto& E : CapturedField)
    {
        const int32 nRow = bMirrorRows ? (Rows - 1 - E.Row) : E.Row;
        const int32 nCol = bMirrorCols ? (Cols - 1 - E.Col) : E.Col;

       if (TargetField->PlaceUnitOnField(nCol, nRow, E.Unit.Get()))
       {
	       RemoveFromField(E.Col, E.Row, true);
       }
    	
    }

    // --- 벤치 이동 ---
    if (bIncludeBench)
    {
        for (const auto& E : CapturedBench)
        {
            int32 NewIndex = E.Index;

            if (bMirrorCols)
            {
                if (bDualBenchThis && bDualBenchTarget && TSize > 0)
                {
                    const int32 capped = E.Index % TSize;
                    NewIndex = TargetField->MirrorBenchIndex(capped); // 좌↔우 반쪽 스왑
                }
                else if (TSize > 0)
                {
                    NewIndex = TSize - 1 - (E.Index % TSize); // 폴백: 전체 역순
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Skip bench mirror: Target BenchSize == 0"));
                }
            }

            if (TargetField->PlaceUnitOnBench(NewIndex, E.Unit.Get()))
                RemoveFromBench(E.Index, true);
        }
    }
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
	BenchSize = 2 * N;
	Bench.SetNum(BenchSize);

	const FVector OwnerLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	const FVector FirstPlayerLoc = OwnerLoc + FVector(BenchTilePlayerLocal.X, BenchTilePlayerLocal.Y, 0);
	const FVector FirstEnemyLoc = OwnerLoc + FVector(BenchTileEnemyLocal.X, BenchTileEnemyLocal.Y, 0);

	for ( int32 i = 0; i < N; ++i)
	{
		Bench[i].Position = FirstPlayerLoc + FVector(0.f, i * BenchStepLocalY, 0.f);
		Bench[i].bIsField = false;
		Bench[i].Unit = nullptr;
	}

	for (int32 i = 0; i < N; ++i)
	{
		const int32 dst = bBenchClockwise ? (N + i) : (N + (N - 1 - i));

		Bench[dst].Position = FirstEnemyLoc + FVector(0.f, i * -BenchStepLocalY, 0.f);
		Bench[dst].bIsField = false;
		Bench[dst].Unit = nullptr;
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

	UE_LOG(LogTemp, Warning, TEXT("World: %s  Best (y=%d,x=%d) Pos:%s  d=%.1f"),
	*WorldLoc.ToString(), BestY, BestX, *Field[IndexOf(BestY,BestX)].Position.ToString(),
	FMath::Sqrt(BestD2));
	
	return true;
	
	
}

bool UPCTileManager::WorldToBench(const FVector& World, int32& OutBenchIndex, float MaxSnapDist) const
{
	OutBenchIndex = INDEX_NONE;
	if (Bench.Num() <= 0)
		return false;

	float BestD2 = TNumericLimits<float>::Max();
	int32 BestIdx = INDEX_NONE;

	for (int32 i = 0; i < Bench.Num(); ++i)
	{
		const float d2 = Dist2_2D(Bench[i].Position, World);
		if (d2 < BestD2)
		{
			BestD2 = d2;
			BestIdx = i;
		}
	}

	if (BestIdx == INDEX_NONE)
		return false;

	const float DefSnap = (TileWidthX > 0.f) ? (TileWidthX * 0.6f) : 120.f;
	const float Snap = (MaxSnapDist > 0.f) ? MaxSnapDist : DefSnap;

	// ★ 제곱거리 비교
	if (BestD2 > (Snap * Snap))
		return false;

	OutBenchIndex = BestIdx;
	return true;
}

bool UPCTileManager::WorldAnyTile(const FVector& World, bool bPreferField, bool& bOutIsField, int32& OutY, int32& OutX,
	int32& OutBenchIndex, FVector& OutSnapPos, float MaxSnapDistField, float MaxSnapDistBench) const
{
	bOutIsField = false;
	OutY = INDEX_NONE;
	OutX = INDEX_NONE;
	OutBenchIndex = INDEX_NONE;
	OutSnapPos = World;

	int32 Y, X, B;
	const bool bField = WorldToField(World, Y, X, MaxSnapDistField);
	const bool bBench = WorldToBench(World, B, MaxSnapDistBench);

	if (!bField && !bBench)
		return false;

	if (bField && bBench)
	{
		const FVector PField = Field[IndexOf(Y, X)].Position;
		const FVector PBench = Bench[B].Position;

		const float df2 = Dist2_2D(PField, World);
		const float db2 = Dist2_2D(PBench, World);

		const bool ChooseField = bPreferField ? (df2 <= db2 * 1.02f) : (df2 < db2);
		if (ChooseField)
		{
			bOutIsField = true;
			OutY = Y; OutX = X;
			OutSnapPos = PField;
		}
		else
		{
			bOutIsField = false;
			OutBenchIndex = B;
			OutSnapPos = PBench;
		}
		return true;
	}

	if (bField)
	{
		bOutIsField = true;
		OutY = Y; OutX = X;
		OutSnapPos = Field[IndexOf(Y, X)].Position;
		return true;
	}

	bOutIsField = false;
	OutBenchIndex = B;
	OutSnapPos = Bench[B].Position;
	return true;

	
}

