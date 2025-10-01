// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/PCCombatPlayerController.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "DataAsset/Player/PCDataAsset_PlayerInput.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCDragComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Shop/PCShopManager.h"
#include "UI/GameResult/PCGameResultWidget.h"
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
	bAutoManageActiveCameraTarget = false;

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

//	APCCombatGameState* CombatGS = GetWorld() 
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

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			/*From*/1.f, /*To*/0.f, /*Duration*/0.001f,
			FLinearColor::Black, /*bFadeAudio*/false, /*bHold*/false);
	}

	// 3) HUD/상점 생성 및 가시성 보정
	EnsureMainHUDCreated();   // 내부에서 AddToViewport
	if (IsValid(PlayerMainWidget))
	{
		PlayerMainWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	GetWorldTimerManager().SetTimer(LoadShop, this, &ThisClass::LoadShopWidget, 3.f , false, 0.1f);
	
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
			Server_StopMovement();
		}
		
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.f, false);
	}
}

void APCCombatPlayerController::OnSetDestinationReleased()
{
	if (FollowTime <= PlayerInputData->ShortPressThreshold && IsLocalController())
	{
		if (APawn* ControlledPawn = GetPawn())
		{
			//UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
			Server_MovetoLocation(CachedDestination);
		}
	}
	
	FollowTime = 0.f;
}

void APCCombatPlayerController::Server_StopMovement_Implementation()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		StopMovement();
	}
}

void APCCombatPlayerController::Server_MovetoLocation_Implementation(const FVector& Destination)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
		Client_MovetoLocation(Destination);
	}
}

void APCCombatPlayerController::Client_MovetoLocation_Implementation(const FVector& Destination)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
	}
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
	GetWorldTimerManager().ClearTimer(LoadShop);
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

TArray<int32> APCCombatPlayerController::GetSameShopSlotIndices(int32 SlotIndex)
{
	TArray<int32> SlotIndices;
	
	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		auto ShopSlots = PS->GetShopSlots();
		auto PurchasedSlots = PS->PurchasedSlots;
		for (int i = 0; i < ShopSlots.Num(); i++)
		{
			if (i == SlotIndex)
			{
				continue;
			}
				
			if (!PurchasedSlots.Contains(i))
			{
				if (ShopSlots[i].UnitTag == ShopSlots[SlotIndex].UnitTag)
				{
					SlotIndices.Add(i);
				}
			}
		}
	}
	
	return SlotIndices;
}

void APCCombatPlayerController::ShopRequest_ShopRefresh(float GoldCost)
{
	if (IsLocalController() && !bIsShopRequestInProgress)
	{
		bIsShopRequestInProgress = true;
		Server_ShopRefresh(GoldCost);
	}
}

void APCCombatPlayerController::ShopRequest_BuyXP()
{
	if (IsLocalController() && !bIsShopRequestInProgress)
	{
		bIsShopRequestInProgress = true;
		Server_BuyXP();
	}
}

void APCCombatPlayerController::ShopRequest_SellUnit()
{
	if (IsLocalController() && !bIsShopRequestInProgress)
	{
		bIsShopRequestInProgress = true;
		Server_SellUnit(CachedHoverUnit.Get());
	}
}

void APCCombatPlayerController::ShopRequest_BuyUnit(int32 SlotIndex)
{
	if (IsLocalController() && !bIsShopRequestInProgress)
	{
		bIsShopRequestInProgress = true;
		Server_BuyUnit(SlotIndex);
	}
}

void APCCombatPlayerController::ShopRequest_ShopLock(bool ShopLockState)
{
	if (IsLocalController())
	{
		Server_ShopLock(ShopLockState);
	}
}

void APCCombatPlayerController::Server_ShopRefresh_Implementation(float GoldCost)
{
	// 라운드 상점 초기화이고, 상점이 잠겨있으면 return
	if (GoldCost == 0 && bIsShopLocked) return;
	
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

	Client_ShopRequestFinished();
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

	Client_ShopRequestFinished();
}

void APCCombatPlayerController::Server_SellUnit_Implementation(APCBaseUnitCharacter* Unit)
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;

	auto PS = GetPlayerState<APCPlayerState>();
	if (!PS) return;

	// 팀 확인
	if (!Unit || Unit->IsActorBeingDestroyed() || Unit->GetTeamIndex() != PS->SeatIndex)
		return;

	if (UPCTileManager* TM = GetTileManager())
	{
		if (TM->GetBenchUnitIndex(Unit) == INDEX_NONE && TM->GetFieldUnitGridPoint(Unit) == FIntPoint::NoneValue)
			return;
	}
	
	
	if (auto ASC = PS->GetAbilitySystemComponent())
	{
		if (Unit && !Unit->IsActorBeingDestroyed())
		{
			FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_SellUnit;
			FGameplayEventData EventData;
			EventData.Instigator = PS;
			EventData.Target = PS;
			EventData.EventTag = GA_Tag;
			EventData.OptionalObject = Unit;

			ASC->HandleGameplayEvent(GA_Tag, &EventData);
			Unit = nullptr;
		}
	}

	Client_ShopRequestFinished();
}

void APCCombatPlayerController::Server_BuyUnit_Implementation(int32 SlotIndex)
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;

	auto PS = GetPlayerState<APCPlayerState>();
	if (!PS) return;

	auto ASC = PS->GetAbilitySystemComponent();
	if (!ASC) return;
	
	auto Board = GS->GetBattleBoardForSeat(PS->SeatIndex);
	if (!Board)
	{
		Board = GS->GetBoardBySeat(PS->SeatIndex);
		if (!Board) return;
	}

	int32 RequiredCount = 0;

	// 벤치가 꽉 찼을 때
	if (Board->GetFirstEmptyBenchIndex(PS->SeatIndex) == INDEX_NONE)
	{
		auto SameSlotIndices = GetSameShopSlotIndices(SlotIndex);
		RequiredCount = GS->GetShopManager()->GetRequiredCountWithFullBench(PS, PS->GetShopSlots()[SlotIndex].UnitTag, SameSlotIndices.Num() + 1);
		
		if (auto AttributeSet = PS->GetAttributeSet())
		{
			// 모두 구매가 가능한지 골드 확인
			if (RequiredCount == 0 || AttributeSet->GetPlayerGold() < PS->GetShopSlots()[SlotIndex].UnitCost * RequiredCount)
			{
				return;
			}

			// 클릭한 슬롯 제외 나머지 구매 처리
			if (RequiredCount >= 1)
			{
				for (int i = 0; i < RequiredCount - 1; ++i)
				{
					Client_SetSlotHidden(SameSlotIndices[i]);
					PS->PurchasedSlots.Add(SameSlotIndices[i]);
				}
			}
			else
			{
				return;
			}
		}
	}

	// 클릭한 슬롯 구매 처리
	FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_BuyUnit;
	FGameplayEventData EventData;
	EventData.Instigator = PS;
	EventData.Target = PS;
	EventData.EventTag = GA_Tag;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewData->HitResult.Location.X = SlotIndex;
	NewData->HitResult.Location.Y = RequiredCount;
	TargetDataHandle.Add(NewData);
	EventData.TargetData = TargetDataHandle;
	
	ASC->HandleGameplayEvent(GA_Tag, &EventData);
	
	Client_SetSlotHidden(SlotIndex);
	Client_ShopRequestFinished();
}

void APCCombatPlayerController::Server_ShopLock_Implementation(bool ShopLockState)
{
	bIsShopLocked = ShopLockState;
}

void APCCombatPlayerController::Client_SetSlotHidden_Implementation(int32 SlotIndex)
{
	ShopWidget->SetSlotHidden(SlotIndex);
}

void APCCombatPlayerController::ClientCameraSetCarousel_Implementation(APCCarouselRing* CarouselRing, int32 SeatIndex, float BlendTime)
{
	if (!IsLocalController() || !IsValid(CarouselRing))
		return;

	if (CurrentCameraType == ECameraFocusType::Carousel && CurrentCarouselSeatIndex == SeatIndex)
		return;

	CurrentCameraType = ECameraFocusType::Carousel;
	CurrentCarouselSeatIndex = SeatIndex;
	CurrentBoardSeatIndex = -1;

	CarouselRing->ApplyCentralViewForSeat(this, SeatIndex, 0.f);
	SwitchCameraWhileBlack(CarouselRing, BlendTime, 0.08f,0.15f,0.5f);
	
}

void APCCombatPlayerController::Client_ShopRequestFinished_Implementation()
{
	bIsShopRequestInProgress = false;
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

void APCCombatPlayerController::SwitchCameraWhileBlack(AActor* NewTarget, float BlendTime, float FadeOutTime,
	float FadeInTime, float HoldBlack)
{
	if (!NewTarget) return;

    // 씬 페이드만으로는 UMG가 안가려지므로 오버레이도 같이 켠다
    EnsureScreenFade();
    SetScreenFadeVisible(true, 0.f); // 투명으로 켠 뒤…
	
    // 카메라: 씬 페이드 아웃
    if (IsLocalController() && PlayerCameraManager)
        PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutTime, FLinearColor::Black, false, true);
	
	ScreenFadeWidget->SetRenderOpacity(1.0f);

    const float CutTime = FadeOutTime + FMath::Max(0.f, HoldBlack);

    // 블랙 유지 중 카메라 블렌드
    GetWorldTimerManager().SetTimer(ThFadeSwitch, [this, NewTarget, BlendTime, FadeInTime]()
    {
        SetViewTargetWithBlend(NewTarget, BlendTime);

        // 블렌드가 끝나면 씬 페이드 인 + 오버레이 알파 내리기
        GetWorldTimerManager().SetTimer(ThFadeIn, [this, FadeInTime]()
        {
            if (PlayerCameraManager)
                PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInTime, FLinearColor::Black, false, false);

            // 오버레이도 천천히 사라지게
            const int32 Steps = 10;
            const float StepDt = FMath::Max(0.01f, FadeInTime / Steps);
            for (int32 i=1;i<=Steps;++i)
            {
                GetWorldTimerManager().SetTimerForNextTick([this, i, Steps]()
                {
                    if (!ScreenFadeWidget) return;
                    const float A = 1.f - (float)i / (float)Steps;
                    ScreenFadeWidget->SetRenderOpacity(A);
                    if (i == Steps) SetScreenFadeVisible(false, 0.f);
                });
            }

        }, FMath::Max(0.01f, BlendTime), false);

    }, CutTime, false);
}

void APCCombatPlayerController::ClearAllCameraTimers()
{
	GetWorldTimerManager().ClearTimer(ThFadeSwitch);
	GetWorldTimerManager().ClearTimer(ThFadeIn);
}

void APCCombatPlayerController::EnsureScreenFade()
{
	if (ScreenFadeWidget || !ScreenFadeClass)
		return;
	ScreenFadeWidget = CreateWidget<UUserWidget>(this, ScreenFadeClass);
	if (ScreenFadeWidget)
	{
		ScreenFadeWidget->AddToViewport(10000);
		ScreenFadeWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APCCombatPlayerController::SetScreenFadeVisible(bool bVisible, float Opacity)
{
	if (!ScreenFadeWidget)
		return;
	ScreenFadeWidget->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	ScreenFadeWidget->SetRenderOpacity(bVisible ? Opacity : 0.f);
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
	if (CurrentCameraType == ECameraFocusType::Board && CurrentBoardSeatIndex == BoardSeatIndex)
		return;

	APCCombatBoard* CombatBoard = FindBoardBySeatIndex(BoardSeatIndex);
	if (!CombatBoard)
		return;

	CurrentCameraType     = ECameraFocusType::Board;
	CurrentBoardSeatIndex = BoardSeatIndex;
	CurrentCarouselSeatIndex = -1;

	SwitchCameraWhileBlack(CombatBoard, Blend,0.08f, 0.15f, 0.5f);
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

		CachedHoverUnit = nullptr;
		ClearHoverHighLight();

		if (ShopWidget)
		{
			ShopWidget->ShowPlayerShopBox();
		}
	}
}

void APCCombatPlayerController::Server_StartDragFromWorld_Implementation(FVector World, int32 DragId)
{

	auto* GS = GetWorld()->GetGameState<APCCombatGameState>();
	const bool bInBattle = GS && (IsBattleTag(GS->GetGameStateTag()) || IsBattleCreep(GS->GetGameStateTag()));
		
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

	if (bInBattle && bIsField)
	{
		Client_DragConfirm(false, DragId, World, nullptr);
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
		Client_CurrentDragUnit(Unit);
	}
	else
	{
		Client_DragConfirm(true, DragId, World, nullptr);
	}
	
}

void APCCombatPlayerController::Server_EndDrag_Implementation(FVector World, int32 DragId)
{
	auto* GS = GetWorld()->GetGameState<APCCombatGameState>();
	const bool bInBattle = GS && (IsBattleTag(GS->GetGameStateTag()) || IsBattleCreep(GS->GetGameStateTag()));
		
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

	if (bField && bInBattle)
	{
		Client_DragEndResult(false, World, DragId, nullptr);
		CurrentDragUnit = nullptr;
		CurrentDragId = 0;
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
			TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, false);
		}
		else if (SrcBench != INDEX_NONE)
		{
			TM->RemoveFromBench(SrcBench, false);
		}

		bool bPlaced = bField ? TM->PlaceUnitOnField(Y,X,Unit,ETileFacing::Auto) : TM->PlaceUnitOnBench(BenchIdx, Unit,ETileFacing::Auto);

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
		TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, false);
		TM->RemoveFromField(Y,X, false);

		bSwapOK = TM->PlaceUnitOnField(Y,X,Unit,ETileFacing::Auto) && TM->PlaceUnitOnField(SrcGrid.Y, SrcGrid.X, DstUnit,ETileFacing::Auto);

		OtherDestWorld = TM->GetTileWorldPosition(SrcGrid.Y, SrcGrid.X);
	}
	else if (!bSrcField && bField)
	{
		// 벤치 -> 필드
		TM->RemoveFromBench(SrcBench, false);
		TM->RemoveFromField(Y,X,false);

		bSwapOK = TM->PlaceUnitOnField(Y,X,Unit,ETileFacing::Auto) && TM->PlaceUnitOnBench(SrcBench, DstUnit,ETileFacing::Auto);
		OtherDestWorld = TM->GetBenchWorldPosition(SrcBench);
	}
	else if (bSrcField && !bField)
	{
		// 필드 -> 벤치
		TM->RemoveFromField(SrcGrid.Y, SrcGrid.X, false);
		TM->RemoveFromBench(BenchIdx, false);

		bSwapOK = TM->PlaceUnitOnBench(BenchIdx, Unit,ETileFacing::Auto) && TM->PlaceUnitOnField(SrcGrid.Y, SrcGrid.X, DstUnit,ETileFacing::Auto);
		OtherDestWorld = TM->GetTileWorldPosition(SrcGrid.Y, SrcGrid.X);
	}
	else
	{
		// 벤치 -> 벤치
		TM->RemoveFromBench(SrcBench, false);
		TM->RemoveFromBench(BenchIdx, false);

		bSwapOK = TM->PlaceUnitOnBench(BenchIdx, Unit,ETileFacing::Auto) && TM->PlaceUnitOnBench(SrcBench, DstUnit,ETileFacing::Auto);

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

	// 임시 코드
	// if (APCHeroUnitCharacter* CurrentHero = Cast<APCHeroUnitCharacter>(CurrentDragUnit))
	// 	CurrentHero->ActionDrag(false);
	
	CurrentDragUnit = nullptr;
	CurrentDragId = 0;
}

void APCCombatPlayerController::Client_DragConfirm_Implementation(bool bOk, int32 DragId, FVector StartSnap, APCHeroUnitCharacter* PreviewHero)
{
	if (!IsLocalController())
		return;

	APCPlayerState* PC = GetPlayerState<APCPlayerState>();
	if (!PC)
		return;
	
	
	if (bOk && PreviewHero)
	{
		if (APCCombatBoard* BattleBoard = FindBoardBySeatIndex(HomeBoardSeatIndex))
		{
			BattleBoard->OnHism(true);

			if (ShopWidget)
			{
				ShopWidget->ShowSellBox();
			}
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

	APCPlayerState* PC = GetPlayerState<APCPlayerState>();
	if (!PC)
		return;

	if (ShopWidget)
	{
		float X, Y;
		UWidgetLayoutLibrary::GetMousePositionScaledByDPI(this, X, Y);
		FVector2D MousePos(X, Y);

		if (ShopWidget->IsScreenPointInSellBox(MousePos))
		{
			Server_SellUnit(CurrentDragUnit.Get());
		}
			
		ShopWidget->ShowPlayerShopBox();	
	}

	
	if (APCCombatBoard* BattleBoard = FindBoardBySeatIndex(HomeBoardSeatIndex))
	{
		BattleBoard->OnHism(false);
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
	const int32 N2 = 2 * TM->BenchSlotsPerSide;
	for (int32 i=0; i<N2; ++i)
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
	auto* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>(): nullptr;

	auto* PS = GetPlayerState<APCPlayerState>();
	if (!GS || !PS) return nullptr;

	const FGameplayTag Cur = GS->GetGameStateTag();

	if (IsBattleTag(Cur))
	{
		if (UPCTileManager* BattleTM = GS->GetBattleTileManagerForSeat(PS->SeatIndex))
			return BattleTM;
	}

	if (APCCombatBoard* Board = GS->GetBoardBySeat(PS->SeatIndex))
		return Board->TileManager;

	return nullptr;
}

bool APCCombatPlayerController::IsAllowBenchIdx(int32 Idx)
{
	UPCTileManager* TM = GetTileManager();
	if (!TM) return false;

	bool bEnemySide = false; 
	int32 Local = INDEX_NONE;
	if (!TM->SplitGlobalBenchIndex(Idx, bEnemySide, Local)) 
		return false;

	// 로컬 범위도 확인(안전장치)
	if (Local < 0 || Local >= TM->BenchSlotsPerSide) 
		return false;

	const APCPlayerState* PS = GetPlayerState<APCPlayerState>();
	const APCCombatBoard* Board = TM->GetCombatBoard();
	if (!PS || !Board) return false;

	const bool bIAmHostOnThisTM = (PS->SeatIndex == Board->BoardSeatIndex);

	// 내가 이 TM의 호스트면 내 벤치는 friendly(= !bEnemySide)
	// 내가 게스트면 내 벤치는 enemy(=  bEnemySide)
	// => XOR 한 줄로 정리
	const bool bMySide = (bEnemySide ^ bIAmHostOnThisTM);
	return bMySide;
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
	

	if (APCBaseUnitCharacter* Unit = bField ? TileManager->GetFieldUnit(Y,X) : TileManager->GetBenchUnit(BenchIdx))
	{
		Client_TileHoverUnit(Unit);
	}
	else
	{
		Client_TileHoverUnit(nullptr);
	}

	
	
}


void APCCombatPlayerController::Server_QueryTileUnit_Implementation(bool bIsField, int32 Y, int32 X, int32 BenchIdx)
{
	UPCTileManager* TM = GetTileManager();
	if (!TM)
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	if (APCBaseUnitCharacter* Unit = bIsField ? TM->GetFieldUnit(Y,X) : TM->GetBenchUnit(BenchIdx))
	{
		Client_TileHoverUnit(Unit);
	}
	else
	{
		Client_TileHoverUnit(nullptr);
	}
	
}

void APCCombatPlayerController::Client_CurrentDragUnit_Implementation(APCBaseUnitCharacter* Unit)
{
	if (!IsLocalController())
		return;

	if (Unit != nullptr)
	{
		CurrentDragUnit = Unit;
	}
	else
	{
		CurrentDragUnit = nullptr;
	}
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

void APCCombatPlayerController::LoadGameResultWidget(int32 Ranking)
{
	if (IsLocalController())
	{
		if (!GameResultWidgetClass) return;
		
		GameResultWidget = CreateWidget<UPCGameResultWidget>(this, GameResultWidgetClass);
		if (!GameResultWidget) return;

		GameResultWidget->SetRanking(Ranking);
		GameResultWidget->OpenMenu();
	}
}