// LobbyMenuWidget.cpp
#include "UI/Lobby/LobbyMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/Overlay.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/SizeBox.h"
#include "Components/ContentWidget.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "GameFramework/GameState/PCLobbyGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Controller/Player/PCLobbyPlayerController.h"
#include "UI/Lobby/PlayerDataWidget.h"
#include "UI/Lobby/CenterPlayerWidget.h" 

void ULobbyMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_ToggleReady) Btn_ToggleReady->OnClicked.AddDynamic(this, &ThisClass::OnClicked_ToggleReady);
	if (Btn_Start)       Btn_Start->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Start);

	CollectSeatSlots();
	SeatWidgets.SetNum(SeatSlots.Num());

	if (bAutoRefresh)
	{
		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().SetTimer(RefreshTimer, this, &ThisClass::RefreshGrid, RefreshIntervalSec, true, 0.f);
		}
	}

	RefreshGrid();
}

void ULobbyMenuWidget::NativeDestruct()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::NativeDestruct();
}

void ULobbyMenuWidget::CollectSeatSlots()
{
	SeatSlots.Reset();

	// 0번: 중앙(필수) — 센터 위젯은 여기 덮어씌움
	if (Slot_Center) SeatSlots.Add(Slot_Center);

	// 1..N: 존재하는 것만 추가
	UPanelWidget* ExtraSlots[] = { Slot_1, Slot_2, Slot_3, Slot_4, Slot_5, Slot_6, Slot_7 };
	for (UPanelWidget* P : ExtraSlots)
	{
		if (IsValid(P)) SeatSlots.Add(P);
	}
}

void ULobbyMenuWidget::BuildOtherVisualOrder(TArray<int32>& OutOrder) const
{
	OutOrder.Reset();
	// 중앙(0)은 항상 로컬 → 제외하고 1..N만 채움
	for (int32 i = 1; i < SeatSlots.Num(); ++i)
	{
		OutOrder.Add(i);
	}
}

void ULobbyMenuWidget::ClearAllSeats()
{
	for (int32 i = 0; i < SeatWidgets.Num(); ++i)
	{
		ClearSeat(i);
	}
}

void ULobbyMenuWidget::ClearSeat(int32 Index)
{
	if (!SeatWidgets.IsValidIndex(Index)) return;

	if (UUserWidget* W = SeatWidgets[Index].Get())
	{
		if (UWidget* Parent = W->GetParent())
		{
			if (UPanelWidget* PP = Cast<UPanelWidget>(Parent))
				PP->RemoveChild(W);
			else if (UContentWidget* PC = Cast<UContentWidget>(Parent))
				PC->SetContent(nullptr);
			else
				W->RemoveFromParent();
		}
	}
	SeatWidgets[Index] = nullptr;
}

UWidget* ULobbyMenuWidget::CreateOrGetSeatWidget(int32 Index)
{
	if (!SeatWidgets.IsValidIndex(Index)) return nullptr;

	if (UUserWidget* Existing = SeatWidgets[Index].Get())
	{
		return Existing;
	}

	UUserWidget* NewWidget = nullptr;

	if (Index == 0 && CenterPlayerWidgetClass)
	{
		NewWidget = CreateWidget<UUserWidget>(this, CenterPlayerWidgetClass); // Center
	}
	else if (PlayerDataWidgetClass)
	{
		NewWidget = CreateWidget<UUserWidget>(this, PlayerDataWidgetClass);   // 일반 타일
	}

	if (NewWidget)
	{
		SeatWidgets[Index] = NewWidget;
		return NewWidget;
	}
	return nullptr;
}

void ULobbyMenuWidget::AddChildStacked(UPanelWidget* Panel, UWidget* Child)
{
	if (!Panel || !Child) return;

	// 이미 부모가 있으면 떼고 온다
	if (UWidget* P = Child->GetParent())
	{
		if (UPanelWidget* PP = Cast<UPanelWidget>(P))           PP->RemoveChild(Child);
		else if (UContentWidget* PC = Cast<UContentWidget>(P))  PC->SetContent(nullptr);
		else                                                    Child->RemoveFromParent();
	}

	// ContentWidget(Border/SizeBox 등) → 기존 컨텐츠 유지 + 위에 Stack
	if (UContentWidget* ContentPanel = Cast<UContentWidget>(Panel))
	{
		if (UWidget* Existing = ContentPanel->GetContent())
		{
			if (UOverlay* AsOverlay = Cast<UOverlay>(Existing))
			{
				AsOverlay->AddChild(Child);
			}
			else
			{
				UOverlay* Stack = NewObject<UOverlay>(this);
				Stack->AddChild(Existing);
				Stack->AddChild(Child);
				ContentPanel->SetContent(Stack);
			}
		}
		else
		{
			ContentPanel->SetContent(Child);
		}
		return;
	}

	// 일반 패널
	Panel->AddChild(Child);
}

void ULobbyMenuWidget::SetSeatOccupied(int32 Index, APCPlayerState* PS)
{
	if (!SeatSlots.IsValidIndex(Index) || !SeatSlots[Index] || !PS) return;

	UWidget* TileW = CreateOrGetSeatWidget(Index);
	if (!TileW) return;

	AddChildStacked(SeatSlots[Index], TileW);

	if (Index == 0)
	{
		if (UCenterPlayerWidget* Center = Cast<UCenterPlayerWidget>(TileW))
		{
			Center->SetPlayer(PS);
			Center->SetVisibility(ESlateVisibility::Visible);
			return;
		}
		// 폴백: 혹시 BP 세팅이 잘못되어 일반 타일이 들어온 경우라도 표시되게
	}

	if (UPlayerDataWidget* Tile = Cast<UPlayerDataWidget>(TileW))
	{
		Tile->SetPlayer(PS);
		Tile->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULobbyMenuWidget::RefreshGrid()
{
	APCLobbyGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCLobbyGameState>() : nullptr;
	if (!GS) return;

	// 슬롯 배열 길이 보정
	if (SeatWidgets.Num() != SeatSlots.Num())
		SeatWidgets.SetNum(SeatSlots.Num());

	// 모두 비우고 다시 채움
	ClearAllSeats();

	// 로컬/타자 분리
	APlayerController* MyPC = GetOwningPlayer();
	const APCPlayerState* LocalPS = MyPC ? MyPC->GetPlayerState<APCPlayerState>() : nullptr;

	TArray<APCPlayerState*> Others;
	for (APlayerState* B : GS->PlayerArray)
	{
		if (auto* PS = Cast<APCPlayerState>(B))
		{
			if (!PS->bIdentified) continue;
			if (PS == LocalPS)    continue;
			Others.Add(PS);
		}
	}
	Others.Sort([](const APCPlayerState& A, const APCPlayerState& B)
	{
		return A.SeatIndex < B.SeatIndex;
	});

	// 0번(중앙)에 로컬 고정 (CenterPlayerCardWidget)
	if (LocalPS && SeatSlots.Num() > 0)
	{
		SetSeatOccupied(0, const_cast<APCPlayerState*>(LocalPS));
	}

	// 나머지 1..N에 타자 배치 (PlayerDataWidget)
	TArray<int32> VisualOrder;
	BuildOtherVisualOrder(VisualOrder);

	const int32 Fill = FMath::Min(Others.Num(), VisualOrder.Num());
	for (int32 i = 0; i < Fill; ++i)
	{
		SetSeatOccupied(VisualOrder[i], Others[i]);
	}

	// 하단 상태/버튼
	if (Tb_ReadyCount)
	{
		Tb_ReadyCount->SetText(
			FText::FromString(FString::Printf(TEXT("Ready %d / %d"),
				GS->NumPlayersReady, GS->NumPlayers)));
	}

	if (Btn_Start)
	{
		const bool bAllReady = (GS->NumPlayers > 0 && GS->NumPlayersReady == GS->NumPlayers);
		Btn_Start->SetIsEnabled(IsLeaderLocal() && bAllReady);
	}
}

void ULobbyMenuWidget::OnClicked_ToggleReady()
{
	if (auto* PC = GetOwningPlayer<APCLobbyPlayerController>())
	{
		if (!PC->IsLocalController()) return;
		const bool bNew = !IsReadyLocal();
		PC->ServerSetReady(bNew);
	}
}

void ULobbyMenuWidget::OnClicked_Start()
{
	if (!IsLeaderLocal()) return;
	if (auto* PC = GetOwningPlayer<APCLobbyPlayerController>())
	{
		PC->ServerRequestStart();
	}
}

bool ULobbyMenuWidget::IsLeaderLocal() const
{
	if (const APlayerController* PC = GetOwningPlayer())
		if (const APCPlayerState* PS = PC->GetPlayerState<APCPlayerState>())
			return PS->bIsLeader;
	return false;
}

bool ULobbyMenuWidget::IsReadyLocal() const
{
	if (const APlayerController* PC = GetOwningPlayer())
		if (const APCPlayerState* PS = PC->GetPlayerState<APCPlayerState>())
			return PS->bIsReady;
	return false;
}