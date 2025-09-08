// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/Component/PCTileManager.h"

#include "Character/UnitCharacter/PCHeroUnitCharacter.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"


// Sets default values for this component's properties
UPCTileManager::UPCTileManager()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	Unit->SetOnCombatBoard(GetCombatBoard());
	Unit->SetActorLocation(Field[i].Position);
	return true;
}

bool UPCTileManager::RemoveFromField(int32 Y, int32 X)
{
	const int32 i = Y * Rows + X;
	if (!Field.IsValidIndex(i))
		return false;
	Field[i].Unit->SetOnCombatBoard(nullptr);
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

FIntPoint UPCTileManager::GetFiledUnitGridPoint(APCBaseUnitCharacter* InUnit) const
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
	Unit->SetOnCombatBoard(CachedCombatBoard.Get());
	Unit->SetActorLocation(Bench[BenchIndex].Position);
	return true;
}

bool UPCTileManager::RemoveFromBench(int32 BenchIndex)
{
	if (!Bench.IsValidIndex(BenchIndex))
		return false;
	Bench[BenchIndex].Unit->SetOnCombatBoard(nullptr);
	Bench[BenchIndex].Unit = nullptr;
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

// 점유자가 떠날 예약이 있으면 통과 후보로 허용
bool UPCTileManager::CanUseNextStep(int32 Y, int32 X, const APCBaseUnitCharacter* InUnit) const
{
	int32 Index;
	if (!IsValidTile(Y,X,Index))
	{
		return true;
	}
	return false;
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

APCCombatBoard* UPCTileManager::GetCombatBoard() const
{
	return CachedCombatBoard.IsValid() ? CachedCombatBoard.Get() : Cast<APCCombatBoard>(GetOwner());
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
    for (int32 col = 0; col < Cols; ++col)
    {
        if (APCBaseUnitCharacter* U = GetFieldUnit(col, row)) 
        {
            CapturedField.Add({col, row, U});
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
            RemoveFromField(E.Col, E.Row);                          
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
                RemoveFromBench(E.Index);
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

void UPCTileManager::DebugDrawTiles(bool bPersistent)
{
	UWorld* W = GetWorld();
	if (!W) return;

	const bool  bPers = bPersistent;
	const float Dur   = DebugDuration;
	const float R     = DebugPointRadius;

	auto DrawPoint = [&](const FVector& P, const FColor& C)
	{
		DrawDebugSphere(W, P, R, 12, C, bPers, Dur);
	};
	auto DrawLabel = [&](const FVector& P, const FString& S)
	{
		if (!bDebugShowIndices) return;
		DrawDebugString(W, P + FVector(0,0,30), S, nullptr, FColor::White, Dur, false, DebugTextScale);
	};

	// --- Field (Rows x Cols) ---
	for (int32 r=0; r<Rows; ++r)
		for (int32 c=0; c<Cols; ++c)
		{
			const int32 i = c * Rows + r;                    // ✅ r*Cols
			if (!Field.IsValidIndex(i)) continue;

			const bool bHasUnit = (Field[i].Unit != nullptr);
			const FColor Col = bHasUnit ? UnitColor : FieldColor;

			DrawDebugSphere(W, Field[i].Position, DebugPointRadius, 12, Col, bPersistent, DebugDuration);

			if (bDebugShowIndices)
			{
				const FString Label = FString::Printf(TEXT("(%d,%d)\n#%d"), c, r, i); // ✅ (Col,Row)
				DrawDebugString(W, Field[i].Position + FVector(0,0,30), Label, nullptr, FColor::White, DebugDuration, false, DebugTextScale);
			}
		}

	// Bench 라벨도 A/B 구분 + 로컬/글로벌 인덱스 표기
	const int32 N = BenchSlotsPerSide;
	for (int32 i=0; i<Bench.Num(); ++i)
	{
		const bool bHasUnit = (Bench[i].Unit != nullptr);
		const bool bSideB   = (N > 0 && i >= N);
		const FColor Col    = bHasUnit ? UnitColor : (bSideB ? BenchBColor : BenchAColor);

		DrawDebugSphere(W, Bench[i].Position, DebugPointRadius, 12, Col, bPersistent, DebugDuration);

		if (bDebugShowIndices)
		{
			const int32 Local = (N > 0) ? (bSideB ? (i - N) : i) : i;
			const TCHAR* Side = bSideB ? TEXT("B") : TEXT("A");
			const FString Label = FString::Printf(TEXT("%s %d\n[%d]"), Side, Local, i);
			DrawDebugString(W, Bench[i].Position + FVector(0,0,30), Label, nullptr, FColor::White, DebugDuration, false, DebugTextScale);
		}
	}
}

