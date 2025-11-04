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
	FVector TWorld = FVector(Loc.X, Loc.Y, 50.f);
	Unit->TeleportTo(TWorld, Rot, false, true);
	BindToUnit(Unit);

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

void UPCTileManager::ClearAll()
{
	for (auto& FieldTile : Field)
	{
		FieldTile.Unit = nullptr;
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
			
	if (FieldGridPoint != FIntPoint::NoneValue)
	{
		RemoveFromField(FieldGridPoint.Y, FieldGridPoint.X, false);
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
			if (Tile.IsOwnedBy(InUnit))
			{
				Tile.Unit = nullptr;
			}
			if (Tile.IsReservedBy(InUnit))
			{
				Tile.ReservedUnit = nullptr;
			}
			return true;
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

bool UPCTileManager::EnsureExclusive(APCBaseUnitCharacter* InUnit)
{
	if (!InUnit) return false;

	const FIntPoint GridPoint = GetFieldUnitGridPoint(InUnit);
	if (GridPoint != FIntPoint::NoneValue)
	{
		RemoveFromField(GridPoint.Y, GridPoint.X, false);
		return true;
	}
	
	return false;
}

TArray<APCBaseUnitCharacter*> UPCTileManager::GetWinnerUnitByTeamIndex(int32 WinnerTeamIndex)
{
	TArray<APCBaseUnitCharacter*> WinnerUnit;
	
	auto AddIfField = [&](APCBaseUnitCharacter* Unit)
	{
		if (!IsValid(Unit)) return;

		if (Unit->GetTeamIndex() == WinnerTeamIndex)
		{
			if (!WinnerUnit.Contains(Unit))
			{
				WinnerUnit.Add(Unit);
			}
		}
	};
	
	for (const auto& T : Field)
	{
		AddIfField(T.Unit);
	}

	return WinnerUnit;
}

TArray<APCBaseUnitCharacter*> UPCTileManager::GetAllAliveUnit()
{
	TArray<APCBaseUnitCharacter*> AliveUnit;

	for (const auto& T : Field)
	{
		if (T.Unit != nullptr)
		{
			AliveUnit.Add(T.Unit);
		}
	}

	return AliveUnit;
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
    	for (int32 y = 0; y < Cols; ++y)
        {
            FString Line;
            Line.Reserve(Cols * 4);
    		for (int32 x = 0; x < Rows; ++x)
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

void UPCTileManager::DebugDrawTiles(float Duration, bool bPersistent, bool bShowIndex, bool bShowYX,
	bool bShowUnit) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Duration을 크게 주면(예: 1e6f) 사실상 계속 보임. bPersistent=true면 지속 라인 배처에 쌓임.
	const float Life = (Duration <= 0.f) ? 10.f : Duration;

	for (int32 i = 0; i < Field.Num(); ++i)
	{
		if (!Field.IsValidIndex(i)) continue;
		const FTile& T = Field[i];
		const FVector P = T.Position;
		const FIntPoint Intpoint = T.UnitIntPoint;

		TArray<FString> Lines;
		if (bShowIndex) Lines.Add(FString::Printf(TEXT("#%d"), i));
		if (bShowYX)
		{
			const int32 Y = Intpoint.Y;
			const int32 X = Intpoint.X;
			Lines.Add(FString::Printf(TEXT("(X=%d,Y=%d)"), X, Y));
		}
	
		const FString Label = FString::Join(Lines, TEXT(" "));

		// 스피어/텍스트
		DrawDebugSphere(World, P, 12.f, 12, FColor::Green, bPersistent, Life, 0, 0.8f);
		DrawDebugString(World, P + FVector(0,-140,40.f), Label, nullptr, FColor::Black, Life, false, 1.0f);
	}
}

void UPCTileManager::DebugClearPersistent() const
{
	if (UWorld* World = GetWorld())
	{
		// ⚠️ 월드의 모든 persistent debug 라인을 지움(다른 디버그도 함께 사라질 수 있음)
		FlushPersistentDebugLines(World);
	}
}

#if WITH_EDITOR
void UPCTileManager::Editor_DrawTilesPersistent()
{
	// 에디터에서도 동작: 맵 열린 상태에서 디테일 버튼 클릭 시 그려지고 유지
	DebugDrawTiles(1e6f /*사실상 영구*/, true, true, true, true);
}

void UPCTileManager::Editor_ClearDebug()
{
	DebugClearPersistent();
}
#endif

FString UPCTileManager::DescribeTileState(int32 Index)
{
	if (!Field.IsValidIndex(Index)) return TEXT("[InvalidIndex]");
	const FTile& T = Field[Index];
	const int32 Y = Index / Rows;
	const int32 X = Index % Rows;
	FString UnitName     = T.Unit ? T.Unit->GetName() : TEXT("null");

	return FString::Printf(
		TEXT("i=%d (Y=%d,X=%d) Pos=(%.1f,%.1f,%.1f) Field=%d | Unit=%s | Free=%d"),
		Index, Y, X,
		T.Position.X, T.Position.Y, T.Position.Z,
		T.bIsField, *UnitName, T.IsFree()
	);
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

	Unit->OnUnitDied.RemoveDynamic(this, &UPCTileManager::OnBoundUnitDied);
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

