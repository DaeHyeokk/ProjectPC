// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/LobbyMenuWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Controller/Player/PCLobbyPlayerController.h"
#include "GameFramework/GameState/PCLobbyGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/Lobby/PlayerDataWidget.h"
#include "Layout/Margin.h"
#include "Styling/SlateBrush.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "Components/BorderSlot.h"
#include "Components/SizeBox.h"

static FSlateBrush MakeBrush(UTexture2D* Texture, const FVector2D Size, const FMargin& Margin, const FLinearColor& Tint)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = Size;
	Brush.Margin    = Margin;   // 9-slice용
	Brush.TintColor = Tint;
	return Brush;
}

static void GetAnchorAndAlignFromOffset(const FVector2D& CenterOffset,
										FAnchors& OutAnchors, FVector2D& OutAlign)
{
	const float Eps = 1.f; // 거의 0이면 '중앙' 취급
	const bool Left   = (CenterOffset.X < -Eps);
	const bool Right  = (CenterOffset.X > +Eps);
	const bool Top    = (CenterOffset.Y < -Eps);
	const bool Bottom = (CenterOffset.Y > +Eps);

	float ax = 0.5f, ay = 0.5f; // 기본 중앙
	if (Left)  ax = 0.f;  else if (Right)  ax = 1.f;
	if (Top)   ay = 0.f;  else if (Bottom) ay = 1.f;

	OutAnchors = FAnchors(ax, ay);
	OutAlign   = FVector2D(ax, ay); // 앵커와 같은 방향으로 정렬(코너 고정)
}

// 앵커/얼라인을 사용해 캔버스에 배치
static void SetAnchoredByCorner(UCanvasPanelSlot* S,
								const FAnchors& Anch, const FVector2D& Align,
								const FVector2D& Size,
								const FVector2D& Margin /*px*/, int32 Z)
{
	S->SetAnchors(Anch);
	S->SetAlignment(Align);

	FMargin Off;
	// 앵커 min=max인 상황에서 Left/Top은 '정렬점'의 위치.
	// 좌/상 정렬이면 +Margin, 우/하 정렬이면 -Margin, 중앙이면 0.
	Off.Left   = (Align.X < 0.25f) ? +Margin.X : (Align.X > 0.75f ? -Margin.X : 0.f);
	Off.Top    = (Align.Y < 0.25f) ? +Margin.Y : (Align.Y > 0.75f ? -Margin.Y : 0.f);
	Off.Right  = Size.X;
	Off.Bottom = Size.Y;

	S->SetOffsets(Off);
	S->SetZOrder(Z);
}

static void SetAnchoredToCenter(UCanvasPanelSlot* CanvasPanelSlot, const FVector2D& CenterOffset, const FVector2D& Size, int32 ZOrder)
{
	CanvasPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	// Offsets: Left/Top=왼쪽/위, Right/Bottom=폭/높이 (앵커 min=max일 때)
	FMargin Off;
	Off.Left   = CenterOffset.X - Size.X * 0.5f;
	Off.Top    = CenterOffset.Y - Size.Y * 0.5f;
	Off.Right  = Size.X;
	Off.Bottom = Size.Y;
	CanvasPanelSlot->SetOffsets(Off);
	CanvasPanelSlot->SetZOrder(ZOrder);
}


void ULobbyMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_ToggleReady)
		Btn_ToggleReady->OnClicked.AddDynamic(this, &ULobbyMenuWidget::OnClicked_ToggleReady);
	if (Btn_Start)
		Btn_Start->OnClicked.AddDynamic(this, &ULobbyMenuWidget::OnClicked_Start);


	SeatWidgets.Empty();
	SeatFrameBorders.Empty();
	SeatEmptyIcons.Empty();
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimer, this, &ULobbyMenuWidget::RefreshGrid, 0.5f, true, 0.0f);
	}

	RefreshGrid();
}

void ULobbyMenuWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::NativeDestruct();
}

void ULobbyMenuWidget::EnsureSeatLayout(int32 NumSeats)
{
	if (!Canvas_Seats || NumSeats <= 0) return;

	// 캔버스 실제 크기 (DPI/ScaleBox 반영)
	const FVector2D CanvasSize = GetCanvasLayoutSize();
	if (CanvasSize.X < 4.f || CanvasSize.Y < 4.f)
	{
		// 첫 몇 틱은 지오메트리가 0일 수 있음 → 다음 틱에 다시 시도
		return;
	}

	SeatLayout.Reset();

	// 중앙 좌표(절대 저장하지 않고, 오프셋만 계산용으로 사용)
	const FVector2D Center = CanvasSize * 0.5f;

	const float W = 450.f, H = 250.f;
	const float MarginX = 20.f, MarginY = 20.f;

	// 화면에서 안 잘리도록 중앙에서 갈 수 있는 최대 거리
	const float MaxDX = FMath::Max(0.f, Center.X - W * 0.5f - MarginX);
	const float MaxDY = FMath::Max(0.f, Center.Y - H * 0.5f - MarginY);

	// 디자인상 원하는 간격과 비교해 더 작은 값 사용
	const float WantDX = W * 1.25f;
	const float WantDY = H * 1.15f;
	const float DX = FMath::Min(WantDX, MaxDX);
	const float DY = FMath::Min(WantDY, MaxDY);

	// ★ 항상 "중앙 기준 오프셋"만 저장
	TArray<FVector2D> Off;
	Off.Add({ 0.f,  0.f}); // 0 center (나)
	Off.Add({-DX, -DY});   // 1 좌상
	Off.Add({ 0.f, -DY});  // 2 상
	Off.Add({+DX, -DY});   // 3 우상
	Off.Add({-DX,  0.f});  // 4 좌
	Off.Add({+DX,  0.f});  // 5 우
	Off.Add({-DX, +DY});   // 6 좌하
	Off.Add({ 0.f, +DY});  // 7 하

	const int32 Use = FMath::Min(NumSeats, Off.Num());
	for (int32 i=0; i<Use; ++i)
	{
		FSeatLayout L;
		L.Size     = FVector2D(W, H);
		L.Position = Off[i];   // 중앙 오프셋만 저장
		SeatLayout.Add(L);
	}
	CenterVisualIndex = 0;
}

void ULobbyMenuWidget::EnsureSeatFrames()
{
	if (!Canvas_Seats || SeatLayout.Num() == 0) return;

    const int32 N = SeatLayout.Num();
    if (SeatFrameBorders.Num() < N) SeatFrameBorders.SetNum(N);
    if (SeatEmptyIcons.Num()   < N) SeatEmptyIcons.SetNum(N);

    for (int32 i = 0; i < N; ++i)
    {
        const FSeatLayout& L = SeatLayout[i];

        // 1) 빈 슬롯 프레임(뒤 레이어)
        if (!SeatFrameBorders[i])
        {
            SeatFrameBorders[i] = NewObject<UBorder>(this);
            Canvas_Seats->AddChild(SeatFrameBorders[i]);
        }
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(SeatFrameBorders[i]->Slot))
        {
            FAnchors Anch; FVector2D Align;
            GetAnchorAndAlignFromOffset(L.Position, Anch, Align);
            const FVector2D FrameMargin(24.f, 20.f);
            SetAnchoredByCorner(S, Anch, Align, L.Size, FrameMargin, /*Z*/2);
        }

        if (SlotFrameTexture)
        {
            FSlateBrush B; B.SetResourceObject(SlotFrameTexture);
            B.ImageSize = L.Size; B.Margin = SlotFrameMargin; B.TintColor = SlotFrameTint;
            SeatFrameBorders[i]->SetBrush(B);
        }
        else
        {
            SeatFrameBorders[i]->SetBrushColor(FLinearColor(0,0,0,0.22f));
        }

        // 2) 플러스 아이콘: 프레임의 컨텐츠로 중앙 배치
        if (!SeatEmptyIcons[i])
        {
            UImage* PlusImg = NewObject<UImage>(this);
            SeatEmptyIcons[i] = PlusImg;

            USizeBox* PlusBox = NewObject<USizeBox>(this);
            PlusBox->SetWidthOverride (40);
            PlusBox->SetHeightOverride(40);
            PlusBox->AddChild(PlusImg);

            SeatFrameBorders[i]->SetContent(PlusBox);

            if (UBorderSlot* BS = Cast<UBorderSlot>(PlusBox->Slot))
            {
                BS->SetHorizontalAlignment(HAlign_Center);
                BS->SetVerticalAlignment  (VAlign_Center);
                BS->SetPadding(FMargin(0));
            }

            if (EmptyPlusTexture)
            {
                FSlateBrush PB; PB.SetResourceObject(EmptyPlusTexture);
                PlusImg->SetBrush(PB);
            }
            else
            {
                PlusImg->SetColorAndOpacity(FLinearColor(1,1,1,0.30f));
            }
        }
    }
}

int32 ULobbyMenuWidget::MapSeatToVisual(int32 ServerSeat, int32 LocalSeat, int32 NumSeats) const
{
	return ServerSeat;
}

void ULobbyMenuWidget::RefreshGrid()
{
	 APCLobbyGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCLobbyGameState>() : nullptr;
    if (!GS || !Canvas_Seats || !PlayerDataWidgetClass) return;

    EnsureSeatLayout(GS->NumSeats);
    EnsureSeatFrames(); // ★ 프레임/플러스 보장

    if (SeatWidgets.Num() < SeatLayout.Num()) SeatWidgets.SetNum(SeatLayout.Num());

	for (UPlayerDataWidget* PlayerDataWidget : SeatWidgets)
	{
		if (PlayerDataWidget)
			PlayerDataWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	TArray<bool> Occupied;
	Occupied.Init(false, SeatLayout.Num());

	const APCPlayerState* LocalPS = nullptr;
	TArray<APCPlayerState*> Others;

	APlayerController* MyPC = GetOwningPlayer<APlayerController>();
	const APCPlayerState* PCPlayerState = MyPC ? MyPC->GetPlayerState<APCPlayerState>() : nullptr;

	for (APlayerState* PSBase : GS->PlayerArray)
	{
		if (APCPlayerState* PS = Cast<APCPlayerState>(PSBase))
		{
			if (!PS->bIdentified) continue;

			if (PS == PCPlayerState)
			{
				LocalPS = PS;            // 나
			}
			else
			{
				Others.Add(PS);          // 타인
			}
		}
	}

    // 1) 나 = 센터
    if (LocalPS && SeatLayout.IsValidIndex(CenterVisualIndex))
    {
        if (UPlayerDataWidget* Tile = EnsureSeatWidget(CenterVisualIndex))
        {
            Tile->SetPlayer(const_cast<APCPlayerState*>(LocalPS));
            Tile->SetVisibility(ESlateVisibility::Visible);
            Occupied[CenterVisualIndex] = true;
        }
    }

    // 2) 타자 = 좌상→우하 순서
	Others.Sort([](const APCPlayerState& A, const APCPlayerState& B){ return A.SeatIndex < B.SeatIndex; });
	TArray<int32> VisualOrder; BuildOtherVisualOrder(VisualOrder);
	for (int32 i=0; i<Others.Num() && i<VisualOrder.Num(); ++i)
	{
		const int32 VI = VisualOrder[i];
		if (UPlayerDataWidget* T = EnsureSeatWidget(VI))
		{ T->SetPlayer(Others[i]); T->SetVisibility(ESlateVisibility::Visible); Occupied[VI]=true; }
	}

	// 프레임/플러스는 '빈칸일 때만' 보이게
	for (int32 i = 0; i < SeatFrameBorders.Num() && i < Occupied.Num(); ++i)
	{
		if (SeatFrameBorders[i])
			SeatFrameBorders[i]->SetVisibility(
				Occupied[i] ? ESlateVisibility::Hidden
							: ESlateVisibility::HitTestInvisible);
	}
	
	// 카운트/버튼은 GS의 복제 값 (식별자만 포함)
	if (Tb_ReadyCount)
		Tb_ReadyCount->SetText(FText::FromString(FString::Printf(TEXT("Ready %d / %d "), GS->NumPlayersReady, GS->NumPlayers)));

	if (Btn_Start)
	{
		const bool bAllReady = (GS->NumPlayers > 0 && GS->NumPlayersReady == GS->NumPlayers);
		Btn_Start->SetIsEnabled(IsLeaderLocal() && bAllReady);
	}
}

UPlayerDataWidget* ULobbyMenuWidget::EnsureSeatWidget(int32 VisualIndex)
{
	if (!SeatLayout.IsValidIndex(VisualIndex)) return nullptr;
	if (SeatWidgets[VisualIndex]) return SeatWidgets[VisualIndex];

	UPlayerDataWidget* Tile = CreateWidget<UPlayerDataWidget>(this, PlayerDataWidgetClass);
	if (!Tile) return nullptr;

	if (UCanvasPanelSlot* S = Canvas_Seats->AddChildToCanvas(Tile))
	{
		PlaceWidgetAt(VisualIndex, Tile);
		SeatWidgets[VisualIndex] = Tile;
		return Tile;
	}
	Tile->RemoveFromParent();
	return nullptr;
}

void ULobbyMenuWidget::PlaceWidgetAt(int32 VisualIndex, UPlayerDataWidget* DataWidget)
{
	if (!SeatLayout.IsValidIndex(VisualIndex) || !DataWidget) return;
	const FSeatLayout& L = SeatLayout[VisualIndex];

	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(DataWidget->Slot))
	{
		FAnchors Anch; FVector2D Align;
		GetAnchorAndAlignFromOffset(L.Position, Anch, Align);

		const FVector2D TileMargin(24.f, 20.f);  // 각 칸의 화면 여백
		SetAnchoredByCorner(S, Anch, Align, L.Size, TileMargin, 10 + VisualIndex);
	}
}

void ULobbyMenuWidget::OnClicked_ToggleReady()
{
	APCLobbyPlayerController* PC = GetOwningPlayer<APCLobbyPlayerController>();
	if (!PC) { UE_LOG(LogTemp, Warning, TEXT("ToggleReady: OwningPlayer nullptr")); return; }
	if (!PC->IsLocalController()) { UE_LOG(LogTemp, Warning, TEXT("ToggleReady: not local controller")); return; }

	const bool bNew = !IsReadyLocal();
	UE_LOG(LogTemp, Warning, TEXT("ToggleReady -> %s  (NetMode=%d)"),
		   bNew ? TEXT("Ready") : TEXT("Unready"), (int32)PC->GetNetMode());

	PC->ServerSetReady(bNew);   // ← 여기서 서버로 올라감
}

void ULobbyMenuWidget::OnClicked_Start()
{
	if (!IsLeaderLocal()) return;
	if (APCLobbyPlayerController* LobbyPlayerController = GetOwningPlayer<APCLobbyPlayerController>())
	{
		LobbyPlayerController->ServerRequestStart();
	}
}

bool ULobbyMenuWidget::IsLeaderLocal() const
{
	if (APlayerController* PlayerController = GetOwningPlayer<APlayerController>())
		if (const APCPlayerState* PCPlayerState = PlayerController->GetPlayerState<APCPlayerState>())
			return PCPlayerState->bIsLeader;
	return false;
}

bool ULobbyMenuWidget::IsReadyLocal() const
{
	if (APlayerController* PlayerController = GetOwningPlayer<APlayerController>())
		if (const APCPlayerState* PCPlayerState = PlayerController->GetPlayerState<APCPlayerState>())
			return PCPlayerState->bIsReady;
	return false;
}

void ULobbyMenuWidget::BuildOtherVisualOrder(TArray<int32>& OutOrder) const
{
	OutOrder.Reset();
	for (int32 i=0;i<SeatLayout.Num();++i) if (i!=CenterVisualIndex) OutOrder.Add(i);
	OutOrder.Sort([this](int32 A,int32 B)
	{
		const FVector2D& PA = SeatLayout[A].Position;
		const FVector2D& PB = SeatLayout[B].Position;
		if (!FMath::IsNearlyEqual(PA.Y, PB.Y)) return PA.Y < PB.Y; // 위쪽 먼저
		return PA.X < PB.X; // 왼쪽 먼저
	});
}

FVector2D ULobbyMenuWidget::GetCanvasLayoutSize() const
{
	if (Canvas_Seats)
	{
		const FGeometry& Geo = Canvas_Seats->GetCachedGeometry();
		const FVector2D Size = Geo.GetLocalSize();
		if (Size.X > 2.f && Size.Y > 2.f)
			return Size; // DPI/Scale 반영된 실제 캔버스 크기
	}

	if (const APlayerController* PC = GetOwningPlayer())
	{
		int32 X = 1920, Y = 1080;
		PC->GetViewportSize(X, Y);
		return FVector2D(X, Y); // 폴백
	}
	return FVector2D(1920.f, 1080.f);
}

