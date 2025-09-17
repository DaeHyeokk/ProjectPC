// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/PCCombatPlayerController.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
//#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/Player/PCDataAsset_PlayerInput.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCDragComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"
#include "UI/Shop/PCShopWidget.h"


APCCombatPlayerController::APCCombatPlayerController()
{
	// 마우스 관련 초기화
	bEnableMouseOverEvents = true;
	bEnableClickEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	bShowMouseCursor = true;

	// 이동 관련 초기화
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;

	// 카메라 관련 초기화
	bAutoManageActiveCameraTarget = true;

	DragComponent = CreateDefaultSubobject<UPCDragComponent>(TEXT("DragComponent"));
}

void APCCombatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(PlayerInputData->BasicInputMappingContext, 0);
	}

	// InputComponent는 Actor.h에서 선언됨
	if (auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(PlayerInputData->SetDestination, ETriggerEvent::Started, this, &APCCombatPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(PlayerInputData->SetDestination, ETriggerEvent::Triggered, this, &APCCombatPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(PlayerInputData->SetDestination, ETriggerEvent::Completed, this, &APCCombatPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(PlayerInputData->SetDestination, ETriggerEvent::Canceled, this, &APCCombatPlayerController::OnSetDestinationReleased);

		EnhancedInputComponent->BindAction(PlayerInputData->BuyXP, ETriggerEvent::Started, this, &APCCombatPlayerController::OnBuyXPStarted);
		EnhancedInputComponent->BindAction(PlayerInputData->ShopRefresh, ETriggerEvent::Started, this, &APCCombatPlayerController::OnShopRefreshStarted);
		EnhancedInputComponent->BindAction(PlayerInputData->SellUnit, ETriggerEvent::Started, this, &APCCombatPlayerController::OnSellUnitStarted);

		// Drag&Drop
		EnhancedInputComponent->BindAction(PlayerInputData->IA_LeftMouse, ETriggerEvent::Started, this, &APCCombatPlayerController::OnMouse_Pressed);
		EnhancedInputComponent->BindAction(PlayerInputData->IA_LeftMouse, ETriggerEvent::Canceled, this, &APCCombatPlayerController::OnMouse_Released);
		EnhancedInputComponent->BindAction(PlayerInputData->IA_LeftMouse, ETriggerEvent::Completed, this, &APCCombatPlayerController::OnMouse_Released);
	}
}

void APCCombatPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ApplyGameInputMode();
	const float Interval = (HoverPollHz > 0.f) ? 1.f / HoverPollHz : 0.066f;
	GetWorldTimerManager().SetTimer(ThHoverPoll, this, &ThisClass::PollHover, Interval, true, 0.1f);
	
}

void APCCombatPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	// 1) 입력 모드 게임용으로 재설정
	ApplyGameInputMode();
	
	// 2) 로비 IMC 제거 후 게임 IMC만 적용
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (auto* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->ClearAllMappings();
			if (PlayerInputData && PlayerInputData->BasicInputMappingContext)
			{
				Subsys->AddMappingContext(PlayerInputData->BasicInputMappingContext, 0);
			}
		}
	}

	// 3) HUD/상점 생성 및 가시성 보정
	EnsureMainHUDCreated();   // 내부에서 AddToViewport
	if (IsValid(PlayerMainWidget))
	{
		PlayerMainWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	//LoadShopWidget();
	
}

void APCCombatPlayerController::OnInputStarted()
{
	FHitResult Hit;
	if (bool bHitSucceeded = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit))
	{
		CachedDestination = Hit.Location;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PlayerInputData->FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f,1.f,1.f), true, true, ENCPoolMethod::None, true);
	}
}

void APCCombatPlayerController::OnSetDestinationTriggered()
{
	if (const UWorld* World = GetWorld())
	{
		FollowTime += World->GetDeltaSeconds();
	}

	FHitResult Hit;
	if (bool bHitSucceeded = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit))
	{
		CachedDestination = Hit.Location;
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		if (FollowTime > PlayerInputData->ShortPressThreshold)
		{
			StopMovement();
		}
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.f, false);
	}
}

void APCCombatPlayerController::OnSetDestinationReleased()
{
	if (FollowTime <= PlayerInputData->ShortPressThreshold)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
	}
	
	FollowTime = 0.f;
}

void APCCombatPlayerController::OnBuyXPStarted()
{
	ShopRequest_BuyXP();
}

void APCCombatPlayerController::OnShopRefreshStarted()
{
	ShopRequest_ShopRefresh(2);
}

void APCCombatPlayerController::OnSellUnitStarted()
{
	ShopRequest_SellUnit();
}

void APCCombatPlayerController::LoadShopWidget()
{
	if (IsLocalController())
	{
		if (!ShopWidgetClass) return;
		
		ShopWidget = CreateWidget<UPCShopWidget>(this, ShopWidgetClass);
		if (!ShopWidget) return;

		ShopRequest_ShopRefresh(0);
		
		ShopWidget->BindToPlayerState(GetPlayerState<APCPlayerState>());
		ShopWidget->OpenMenu();
	}
}

void APCCombatPlayerController::ShopRequest_ShopRefresh(float GoldCost)
{
	if (IsLocalController())
	{
		Server_ShopRefresh(GoldCost);
	}
}

void APCCombatPlayerController::ShopRequest_BuyXP()
{
	if (IsLocalController())
	{
		Server_BuyXP();
	}
}

void APCCombatPlayerController::ShopRequest_SellUnit()
{
	if (IsLocalController())
	{
		Server_SellUnit();
	}
}

void APCCombatPlayerController::ShopRequest_BuyUnit(int32 SlotIndex)
{
	if (IsLocalController())
	{
		Server_BuyUnit(SlotIndex);
	}
}

void APCCombatPlayerController::Server_ShopRefresh_Implementation(float GoldCost)
{
	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_ShopRefresh;
			FGameplayEventData EventData;
			EventData.Instigator = PS;
			EventData.Target = PS;
			EventData.EventTag = GA_Tag;
			EventData.EventMagnitude = GoldCost;

			ASC->HandleGameplayEvent(GA_Tag, &EventData);
		}
	}
}

void APCCombatPlayerController::Server_BuyXP_Implementation()
{
	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(PlayerGameplayTags::Player_GA_Shop_BuyXP));
		}
	}
}

void APCCombatPlayerController::Server_SellUnit_Implementation()
{
	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			if (OverlappedUnit)
			{
				FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_SellUnit;
				FGameplayEventData EventData;
				EventData.Instigator = PS;
				EventData.Target = PS;
				EventData.EventTag = GA_Tag;
				EventData.OptionalObject = OverlappedUnit;

				ASC->HandleGameplayEvent(GA_Tag, &EventData);
				OverlappedUnit = nullptr;
			}
		}
	}
}

void APCCombatPlayerController::Server_BuyUnit_Implementation(int32 SlotIndex)
{
	if (auto GS = GetWorld()->GetGameState<APCCombatGameState>())
	{
		if (auto PS = GetPlayerState<APCPlayerState>())
		{
			if (auto Board = GS->GetBoardBySeat(PS->SeatIndex))
			{
				if (Board->GetFirstEmptyBenchIndex() == -1)
				{
					return;
				}
			}
			
			if (auto ASC = PS->GetAbilitySystemComponent())
			{
				FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_BuyUnit;
				FGameplayEventData EventData;
				EventData.Instigator = PS;
				EventData.Target = PS;
				EventData.EventTag = GA_Tag;
				EventData.EventMagnitude = static_cast<float>(SlotIndex);
				
				ASC->HandleGameplayEvent(GA_Tag, &EventData);
				
				SetSlotHidden(SlotIndex);
			}
		}
	}
}

void APCCombatPlayerController::SetSlotHidden_Implementation(int32 SlotIndex)
{
	ShopWidget->SetSlotHidden(SlotIndex);
}

void APCCombatPlayerController::SetOverlappedUnit_Implementation(APCHeroUnitCharacter* NewUnit)
{
	OverlappedUnit = NewUnit;
}

void APCCombatPlayerController::ClientCameraSetCarousel_Implementation(APCCarouselRing* CarouselRing, int32 SeatIndex, float BlendTime)
{
	if (!IsLocalController() || !IsValid(CarouselRing))
		return;
	//CarouselRing->ApplyCentralViewForSeat(this, CurrentFocusedSeatIndex, 0.f);
	//FadeSwitchCamera(CarouselRing, 0.1f,0.05f,0,0.1f, false);
	SetViewTargetWithBlend(CarouselRing,0);
	
}

void APCCombatPlayerController::SetBoardSpringArmPresets()
{
	
	UWorld* World = GetWorld();
	if (!World) return;
	for (TActorIterator<APCCombatBoard> It(World); It; ++It)
	{
		APCCombatBoard* Board = *It;
		if (!IsValid(Board)) continue;
		
		if (const bool bIsHome = (Board->BoardSeatIndex == HomeBoardSeatIndex))
		{
			Board->ApplyClientHomeView();
		}
		else
		{
			Board->ApplyClientMirrorView();
		}
	}
}

APCCombatBoard* APCCombatPlayerController::FindBoardBySeatIndex(int32 BoardSeatIndex) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	for (TActorIterator<APCCombatBoard> It(World); It; ++It)
	{
		if (It->BoardSeatIndex == BoardSeatIndex)
			return *It;
	}
	return nullptr;
}

void APCCombatPlayerController::FadeSwitchCamera(AActor* NewTarget, float FadeOutTime, float HoldBlack, float BlendTime,
	float FadeInTime, bool bShowHUDAfter)
{
	if (!IsLocalController() || !NewTarget)
	{
		SetViewTargetWithBlend(NewTarget, BlendTime);
		if (bShowHUDAfter)
		{
			PlayerMainWidget->SetVisibility(ESlateVisibility::Visible);
			ShowWidget();
		}
		else
		{
			HideWidget();
		}
		return;
	}

	PlayerMainWidget->SetVisibility(ESlateVisibility::Hidden);

	GetWorldTimerManager().ClearTimer(ThFadeSwitch);
	GetWorldTimerManager().ClearTimer(ThFadeIn);
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.f,1.f,FadeOutTime,FLinearColor::Black,
		false, true);
	}
	
	const float CutTime = FadeOutTime + HoldBlack;

	GetWorldTimerManager().SetTimer(
		ThFadeSwitch,
		[this, NewTarget, BlendTime, FadeInTime, bShowHUDAfter]()
		{
			// 하드 컷으로 숨기고 싶으면 BlendTime=0로 호출하거나:
			// PlayerCameraManager->SetGameCameraCutThisFrame();

			SetViewTargetWithBlend(NewTarget, BlendTime);

			// 3) 블랙을 유지한 채 블렌드가 끝나기를 기다렸다가 페이드 인 시작
			GetWorldTimerManager().SetTimer(
				ThFadeIn,
				[this, FadeInTime, bShowHUDAfter]()
				{
					if (PlayerCameraManager)
					{
						PlayerCameraManager->StartCameraFade(
							1.f, 0.f, FadeInTime,
							FLinearColor::Black,
							/*bFadeAudio*/false,
							/*bHoldWhenFinished*/false
						);
					}
					FTimerHandle VisibleWidget;
					GetWorldTimerManager().SetTimer(
						VisibleWidget,[this]()
						{
							PlayerMainWidget->SetVisibility(ESlateVisibility::Visible);
						}, FMath::Max(0.f, FadeInTime), false);
				},
				FMath::Max(0.f, BlendTime),
				false
			);
		},
		CutTime,
		false
	);
	
}

void APCCombatPlayerController::EnsureMainHUDCreated()
{
	if (!IsLocalController()) return;
	if (!PlayerMainWidgetClass) { UE_LOG(LogTemp, Warning, TEXT("HUD Class NULL")); return; }
	if (!ShopWidgetClass)
		return;
	

	// 이미 있으면 보장만
	if (!IsValid(PlayerMainWidget))
	{
		PlayerMainWidget = CreateWidget<UPCPlayerMainWidget>(this, PlayerMainWidgetClass);
		if (!PlayerMainWidget) { UE_LOG(LogTemp, Warning, TEXT("CreateWidget failed")); return; }

		// 뷰포트에 항상 붙여둔다 (단 1회)
		PlayerMainWidget->AddToViewport();
		PlayerMainWidget->InitAndBind();
		
		// UMG Construct 타이밍 대비 다음 프레임 보정 (옵션)
		FTimerHandle Th;
		GetWorld()->GetTimerManager().SetTimer(Th, [this]()
		{
			if (IsValid(PlayerMainWidget)) PlayerMainWidget->InitAndBind();
		}, 0.f, false);
	}
	else
	{
		// 재보장: 뷰포트에 없으면 붙이고, 바인딩 최신화
		if (!PlayerMainWidget->IsInViewport())
			PlayerMainWidget->AddToViewport();
		PlayerMainWidget->InitAndBind();
		
	}

	if (!IsValid(ShopWidget))
	{
		ShopWidget = CreateWidget<UPCShopWidget>(this, ShopWidgetClass);
		if (!ShopWidget)
			return;
		ShopWidget->AddToViewport();
	}
	else
	{
		if (!ShopWidget->IsInViewport())
		{
			ShopWidget->AddToViewport();
		}
	}
}

void APCCombatPlayerController::ShowWidget()
{
	if (!IsLocalController() || !IsValid(ShopWidget))
		return;
	ShopWidget->SetVisibility(ESlateVisibility::Visible);
	
}

void APCCombatPlayerController::HideWidget()
{
	if (!IsLocalController() || !IsValid(ShopWidget))
		return;
	ShopWidget->SetVisibility(ESlateVisibility::Hidden);
}

void APCCombatPlayerController::ApplyGameInputMode()
{
	FInputModeGameAndUI Mode;

	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	// 커서/마우스 이벤트 보이도록
	bShowMouseCursor      = true;
	bEnableClickEvents    = true;
	bEnableMouseOverEvents= true;
}


void APCCombatPlayerController::Client_ShowWidget_Implementation()
{
	ShowWidget();
}

void APCCombatPlayerController::Client_HideWidget_Implementation()
{
	HideWidget();
}

void APCCombatPlayerController::ClientSetHomeBoardIndex_Implementation(int32 InHomeBoardIdx)
{
	if (bBoardPresetInitialized && HomeBoardSeatIndex == InHomeBoardIdx)
	{
		return;
	}
	HomeBoardSeatIndex = InHomeBoardIdx;
	SetBoardSpringArmPresets();
	bBoardPresetInitialized = true;
}

void APCCombatPlayerController::ClientFocusBoardBySeatIndex_Implementation(int32 BoardSeatIndex,
	bool bBattle, float Blend)
{
	if (CurrentFocusedSeatIndex == BoardSeatIndex)
	{
		return;
	}

	if (APCCombatBoard* CombatBoard = FindBoardBySeatIndex(BoardSeatIndex))
	{
		const bool bShowHUDAfter = bBattle;
		FadeSwitchCamera(CombatBoard,0.5f,0.05f,Blend,0.5f,bShowHUDAfter);
		CurrentFocusedSeatIndex = BoardSeatIndex;
	}
}



void APCCombatPlayerController::OnMouse_Pressed()
{
	if (IsLocalController())
	{
		if (DragComponent)
			DragComponent->OnMouse_Pressed(this);
	}
	
}

void APCCombatPlayerController::OnMouse_Released()
{
	if (IsLocalController())
	{
		if (DragComponent)
			DragComponent->OnMouse_Released(this);
	}
	
}

void APCCombatPlayerController::Server_StartDragFromWorld_Implementation(FVector World, int32 DragId)
{
	UPCTileManager* TM = GetTileManager();
	if (!TM)
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	bool bIsField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;

	if (!TM->WorldAnyTile(World, true, bIsField, Y, X, BenchIdx, Snap))
	{
		Client_DragConfirm(false,DragId, World,nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = bIsField ? TM->GetFieldUnit(Y,X) : TM->GetBenchUnit(BenchIdx);
	
	if (!Unit || !CanControlUnit(Unit))
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	CurrentDragId = DragId;
	CurrentDragUnit = Unit;

	if (APCHeroUnitCharacter* PreviewUnit = Cast<APCHeroUnitCharacter>(Unit))
	{
		Client_DragConfirm(true, DragId, Snap, PreviewUnit);
	}
	else
	{
		Client_DragConfirm(true, DragId, World, nullptr);
	}
	
}

void APCCombatPlayerController::Server_EndDrag_Implementation(FVector World, int32 DragId)
{
	if (DragId != CurrentDragId || !CurrentDragUnit.IsValid())
	{
		Client_DragEndResult(false, World, DragId, nullptr);
		return;
	}

	UPCTileManager* TM = GetTileManager();
	if (!TM)
	{
		Client_DragEndResult(false, World, DragId, nullptr);
		return;
	}

	bool bField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;
	
	if (!TM->WorldAnyTile(World, true, bField, Y, X, BenchIdx, Snap))
	{
		Client_DragEndResult(false, World, DragId, nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = CurrentDragUnit.Get();

	// 1) 유효성 판단
	bool bValid = false;
	APCBaseUnitCharacter* DstUnit = nullptr;
	
	if (bField)
	{
		if (IsAllowFieldY(Y) && TM->IsInRange(Y,X))
		{
			DstUnit = TM->GetFieldUnit(Y,X);
			bValid = (DstUnit == nullptr) || (DstUnit == Unit) || CanControlUnit(DstUnit);
		}
	}
	else
	{
		if (IsAllowBenchIdx(BenchIdx))
		{
			DstUnit = TM->GetBenchUnit(BenchIdx);
			bValid = (DstUnit == nullptr || DstUnit == Unit || CanControlUnit(DstUnit));
		}
		
	}

	if (!bValid)
	{
		Client_DragEndResult(false, Snap, DragId, nullptr);
		return;
	}

	const FIntPoint SrcGrid = TM->GetFieldUnitGridPoint(Unit);
	const bool bSrcField = (SrcGrid != FIntPoint::NoneValue);
	const int32 SrcBench = bSrcField ? INDEX_NONE : TM->GetBenchUnitIndex(Unit);

	if (DstUnit == nullptr || DstUnit == Unit)
	{
		if (bSrcField)
		{
			TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, true);
		}
		else if (SrcBench != INDEX_NONE)
		{
			TM->RemoveFromBench(SrcBench, true);
		}

		bool bPlaced = bField ? TM->PlaceUnitOnField(Y,X,Unit) : TM->PlaceUnitOnBench(BenchIdx, Unit);

		if (bPlaced)
		{
			Multicast_LerpMove(Unit, Snap, LerpDuration);
			Client_DragEndResult(true, Snap, DragId, Cast<APCHeroUnitCharacter>(Unit));
		}
		else
		{
			Client_DragEndResult(false, Snap, DragId, Cast<APCHeroUnitCharacter>(Unit));
		}

		CurrentDragUnit = nullptr;
		CurrentDragId = 0;
		return;
	}

	if (!CanControlUnit(DstUnit))
	{
		Client_DragEndResult(false, Snap, DragId, nullptr);
	}

	bool bSwapOK = false;
	FVector OtherDestWorld = FVector::ZeroVector;

	if (bSrcField && bField)
	{
		// 필드 -> 필드
		TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, true);
		TM->RemoveFromField(Y,X, true);

		bSwapOK = TM->PlaceUnitOnField(Y,X,Unit) && TM->PlaceUnitOnField(SrcGrid.Y, SrcGrid.X, DstUnit);

		OtherDestWorld = TM->GetTileWorldPosition(SrcGrid.Y, SrcGrid.X);
	}
	else if (!bSrcField && bField)
	{
		// 벤치 -> 필드
		TM->RemoveFromBench(SrcBench, true);
		TM->RemoveFromField(Y,X,true);

		bSwapOK = TM->PlaceUnitOnField(Y,X,Unit) && TM->PlaceUnitOnBench(SrcBench, DstUnit);
		OtherDestWorld = TM->GetBenchWorldPosition(SrcBench);
	}
	else if (bSrcField && !bField)
	{
		// 필드 -> 벤치
		TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, true);
		TM->RemoveFromBench(BenchIdx, true);

		bSwapOK = TM->PlaceUnitOnBench(BenchIdx, Unit) && TM->PlaceUnitOnField(SrcGrid.Y, SrcGrid.X, DstUnit);
		OtherDestWorld = TM->GetTileWorldPosition(SrcGrid.Y, SrcGrid.X);
	}
	else
	{
		// 벤치 -> 벤치
		TM->RemoveFromBench(SrcBench, true);
		TM->RemoveFromBench(BenchIdx, true);

		bSwapOK = TM->PlaceUnitOnBench(BenchIdx, Unit) && TM->PlaceUnitOnBench(SrcBench, DstUnit);

		OtherDestWorld = TM->GetBenchWorldPosition(SrcBench);
	}

	if (bSwapOK)
	{
		Multicast_LerpMove(Unit, Snap, LerpDuration);
		Multicast_LerpMove(DstUnit, OtherDestWorld, LerpDuration);
		Client_DragEndResult(true, Snap, DragId, Cast<APCHeroUnitCharacter>(Unit));
	}
	else
	{
		Client_DragEndResult(false, World, DragId, nullptr);
	}
	
	CurrentDragUnit = nullptr;
	CurrentDragId = 0;
}

void APCCombatPlayerController::Client_DragConfirm_Implementation(bool bOk, int32 DragId, FVector StartSnap, APCHeroUnitCharacter* PreviewHero)
{
	if (!IsLocalController())
		return;

	if (bOk)
	{
		if (const APCCombatBoard* Board = FindBoardBySeatIndex(HomeBoardSeatIndex))
		{
			Board -> OnHism(true);
		}
	}
	
	if (DragComponent)
	{
		DragComponent->OnServerDragConfirm(bOk, DragId, StartSnap, PreviewHero);
	}
}

void APCCombatPlayerController::Client_DragEndResult_Implementation(bool bSuccess, FVector FinalSnap, int32 DragId, APCHeroUnitCharacter* PreviewUnit)
{
	if (!IsLocalController())
		return;
	
	if (const APCCombatBoard* Board = FindBoardBySeatIndex(HomeBoardSeatIndex))
	{
		Board -> OnHism(false);
	}
	
	if (DragComponent)
	{
		DragComponent->OnServerDragEndResult(bSuccess, FinalSnap, DragId, PreviewUnit);
	}
}

bool APCCombatPlayerController::CanControlUnit(const APCBaseUnitCharacter* Unit) const
{
	if (!Unit)
		return false;
	if (const APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>())
	{
		return (Unit->GetTeamIndex() == PCPlayerState->SeatIndex);
	}
	return false;
}

bool APCCombatPlayerController::RemoveFromCurrentSlot(UPCTileManager* TM, APCBaseUnitCharacter* Unit) const
{
	if (!TM || !Unit) return false;

	// 필드에서 찾기
	for (int32 x=0; x<TM->Rows; ++x)
	{
		for (int32 y=0; y<TM->Cols; ++y)
		{
			if (TM->GetFieldUnit(y, x) == Unit)
			{
				TM->RemoveFromField(y, x, /*bPreserveUnitBoard=*/true);
				return true;
			}
		}
	}

	// 벤치에서 찾기
	for (int32 i=0; i<TM->BenchSize; ++i)
	{
		if (TM->GetBenchUnit(i) == Unit)
		{
			TM->RemoveFromBench(i, /*bPreserveUnitBoard=*/true);
			return true;
		}
	}
	return false;
}


void APCCombatPlayerController::Multicast_LerpMove_Implementation(APCBaseUnitCharacter* Unit, FVector Dest,
	float Duration)
{
	if (!Unit || HasAuthority())
		return;
	if (USceneComponent* RootComp = Unit->GetRootComponent())
	{
		UKismetSystemLibrary::MoveComponentTo(RootComp, Dest, RootComp->GetComponentRotation(),
			true,true,Duration,false,EMoveComponentAction::Move,FLatentActionInfo());
	}
}

UPCTileManager* APCCombatPlayerController::GetTileManager() const
{
	APCPlayerState* PS = GetPlayerState<APCPlayerState>();
	if (!PS) return nullptr;

	APCCombatBoard* Board = FindBoardBySeatIndex(PS->SeatIndex);
	return Board ? Board->TileManager : nullptr;
}

void APCCombatPlayerController::SetHoverHighLight(APCBaseUnitCharacter* NewUnit)
{
	if (LastHoverUnit.Get() == NewUnit)
		return;
	if (LastHoverUnit.IsValid())
	{
		LastHoverUnit->SetOutlineEnabled(false);
	}

	LastHoverUnit = NewUnit;

	if (LastHoverUnit.IsValid())
	{
		LastHoverUnit->SetOutlineEnabled(true);
	}
}

void APCCombatPlayerController::ClearHoverHighLight()
{
	if (LastHoverUnit.IsValid())
	{
		LastHoverUnit->SetOutlineEnabled(false);
	}

	LastHoverUnit = nullptr;
}

void APCCombatPlayerController::PollHover()
{
	if (DragComponent && DragComponent->IsDraggingOrPending())
		return;

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		ClearHoverHighLight();
		return;
	}

	if (CachedHoverUnit == nullptr)
	{
		ClearHoverHighLight();
	}

	static FVector LastWorld(FLT_MAX, FLT_MAX, FLT_MAX);
	static double LastTime = 0.0;
	const double Now = GetWorld() ? GetWorld()->TimeSeconds:0.0;
	if ((Now - LastTime) < 0.05 && FVector::DistSquared2D(LastWorld, HitResult.Location) < FMath::Square(8.f))
		return;

	LastWorld = HitResult.Location;
	LastTime = Now;

	Server_QueryHoverFromWorld(HitResult.Location);
}

void APCCombatPlayerController::Server_QueryHoverFromWorld_Implementation(const FVector& World)
{
	if (World.IsNearlyZero())
	{
		Client_TileHoverUnit(nullptr);
		return;
	}
	
	UPCTileManager* TileManager = GetTileManager();
	if (!TileManager)
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	bool bField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;
	if (!TileManager->WorldAnyTile(World, true, bField, Y, X, BenchIdx, Snap))
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = bField ? TileManager->GetFieldUnit(Y,X) : TileManager->GetBenchUnit(BenchIdx);
	

	Client_TileHoverUnit(Unit);
}


void APCCombatPlayerController::Server_QueryTileUnit_Implementation(bool bIsFiled, int32 Y, int32 X, int32 BenchIdx)
{
	UPCTileManager* TM = GetTileManager();
	if (!TM)
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = bIsFiled ? TM->GetFieldUnit(Y,X) : TM->GetBenchUnit(BenchIdx);
	Client_TileHoverUnit(Unit);
}

void APCCombatPlayerController::Client_TileHoverUnit_Implementation(APCBaseUnitCharacter* Unit)
{
	if (!IsLocalController())
		return;

	if (bKeepDragHighlight)
		return;

	if (Unit != nullptr)
	{
		CachedHoverUnit = Unit;
		SetHoverHighLight(Unit);
	}
	else
	{
		CachedHoverUnit = nullptr;
		ClearHoverHighLight();
	}
	
	// 안전하게 이름/팀 추출
	const bool bValid = IsValid(Unit);
	const FString UnitName   = bValid ? Unit->GetName() : TEXT("NULL");
	const int32   TeamIndex  = bValid ? Unit->GetTeamIndex() : -1;
	const ENetMode NetMode   = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
	const float    Now       = GetWorld() ? GetWorld()->TimeSeconds : 0.f;

	UE_LOG(LogTemp, Log, TEXT("[Client_TileHoverUnit] t=%.3f NetMode=%d IsLocal=%d Unit=%s Ptr=%p Team=%d"),
		Now, (int32)NetMode, (int32)IsLocalController(), *UnitName, Unit, TeamIndex);
	
}