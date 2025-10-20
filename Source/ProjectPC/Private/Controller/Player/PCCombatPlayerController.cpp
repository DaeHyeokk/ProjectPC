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
#include "Component/PCSynergyComponent.h"
#include "DataAsset/Player/PCDataAsset_PlayerInput.h"
#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCDragComponent.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Shop/PCShopManager.h"
#include "UI/GameResult/PCGameResultWidget.h"
#include "UI/Item/PCPlayerInventoryWidget.h"
#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"
#include "UI/Shop/PCShopWidget.h"
#include "UI/Unit/PCHeroStatusHoverPanel.h"


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

void APCCombatPlayerController::Client_RequestIdentity_Implementation()
{
	// 플레이어 아이디 셋팅
	if (UProfileSubsystem* Profile = GetGameInstance()->GetSubsystem<UProfileSubsystem>())
	{
		const FString Name = Profile->GetDisplayName();
		if (!Name.IsEmpty())
		{
			ServerSubmitIdentity(Name);
		}
	}
}

void APCCombatPlayerController::ServerSubmitIdentity_Implementation(const FString& InDisplayName)
{
	if (APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>())
	{
		PCPlayerState->SetDisplayName_Server(InDisplayName);
	}
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

	if (APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
	{
		PCGameState->OnLeaderBoardReady.AddUObject(this, &APCCombatPlayerController::LoadMainWidget);
	}
	else
	{
		// 안전책: 월드 틱 이후 GameState를 다시 확인
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (APCCombatGameState* GS2 = GetWorld()->GetGameState<APCCombatGameState>())
			{
				GS2->OnLeaderBoardReady.AddUObject(this, &APCCombatPlayerController::LoadMainWidget);
			}
		});
	}

	// 마우스 호버 풀링함수
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

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			/*From*/1.f, /*To*/0.f, /*Duration*/0.001f,
			FLinearColor::Black, /*bFadeAudio*/false, /*bHold*/false);
	}

	if (PlayerMainWidget)
	{
		PlayerMainWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>();
		UPCShopWidget* ShopWidgetRef = PlayerMainWidget->GetShopWidget();
		UPCPlayerInventoryWidget* InventoryWidget = PlayerMainWidget->GetInventoryWidget();
		
		if (!PCPlayerState) return;
		
		if (!ShopWidgetRef) return;
		
		ShopWidgetRef->BindToPlayerState(PCPlayerState);
		ShopWidgetRef->InitWithPC(this);
		
		if (!InventoryWidget) return;
		InventoryWidget->BindToPlayerState(PCPlayerState);
	}
}

void APCCombatPlayerController::OnInputStarted()
{

	UPCHeroStatusHoverPanel* HeroStatusWidget =  PlayerMainWidget->GetHeroStatusWidget();
	if (!HeroStatusWidget)
		return;

	if (CachedCheckStatusUnit.Get())
	{
		if (APCHeroUnitCharacter* HoverUnit = Cast<APCHeroUnitCharacter>(CachedCheckStatusUnit.Get()))
		{
			HeroStatusWidget->ShowPanelForHero(HoverUnit);
			return;
		}
	}
	
	FHitResult Hit;
	if (bool bHitSucceeded = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit))
	{
		CachedDestination = Hit.Location;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PlayerInputData->FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f,1.f,1.f), true, true, ENCPoolMethod::None, true);
	}

	HeroStatusWidget->HidePanel();
	
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
		FVector CurrentLocation = ControlledPawn->GetActorLocation();
		FVector Direction = CachedDestination - CurrentLocation;
			     
		FRotator TargetRotation = Direction.Rotation();
		FRotator NewRotation = FRotator(0.0f, TargetRotation.Yaw, 0.0f);
			     
		ControlledPawn->SetActorRotation(NewRotation);

		Server_SetRotation(CachedDestination);
		
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
		Server_MovetoLocation(CachedDestination);
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

void APCCombatPlayerController::Server_SetRotation_Implementation(const FVector& Destination)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		FVector CurrentLocation = ControlledPawn->GetActorLocation();
		FVector Direction = Destination - CurrentLocation;
			     
		FRotator TargetRotation = Direction.Rotation();
		FRotator NewRotation = FRotator(0.0f, TargetRotation.Yaw, 0.0f);
			     
		ControlledPawn->SetActorRotation(NewRotation);
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
	if (IsLocalController())
	{
		if (!ShopWidgetClass) return;
		
		ShopWidget = CreateWidget<UPCShopWidget>(this, ShopWidgetClass);
		if (!ShopWidget) return;

		ShopRequest_ShopRefresh(0);

		if (APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>())
		{
			ShopWidget->BindToPlayerState(PCPlayerState);
			ShopWidget->OpenMenu();
		}
		else
		{
			// 안전책: 월드 틱 이후 GameState를 다시 확인
			GetWorldTimerManager().SetTimerForNextTick([this]()
			{
				if (APCPlayerState* PCPS2 = GetPlayerState<APCPlayerState>())
				{
					ShopWidget->BindToPlayerState(PCPS2);
					ShopWidget->OpenMenu();
				}
			});
		}
	}
}

void APCCombatPlayerController::LoadMainWidget()
{
	if (IsLocalController())
	{
		EnsureMainHUDCreated();
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
	GetPlayerState<APCPlayerState>()->GetPlayerInventory()->AddItemToInventory(ItemTags::Item_Type_Base_BFSword);
	GetPlayerState<APCPlayerState>()->GetPlayerInventory()->AddItemToInventory(ItemTags::Item_Type_Base_BFSword);
	
	// 라운드 상점 초기화이고, 상점이 잠겨있으면 return
	if (GoldCost == 0 && bIsShopLocked)
	{
		Client_ShopRequestFinished();
		return;
	}
	
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
	if (!GS)
	{
		Client_ShopRequestFinished();
		return;
	}

	auto PS = GetPlayerState<APCPlayerState>();
	if (!PS)
	{
		Client_ShopRequestFinished();
		return;
	}

	// 팀 확인
	if (!Unit || Unit->IsActorBeingDestroyed() || Unit->GetTeamIndex() != PS->SeatIndex)
	{
		Client_ShopRequestFinished();
		return;
	}

	// ✅ PlayerBoard 기준으로 소유/존재 확인
	APCPlayerBoard* PB = GetPlayerBoard();
	if (!IsValid(PB))
	{
		Client_ShopRequestFinished();
		return;
	}
	
	const bool bOnMyBoard =
		(PB->GetBenchUnitIndex(Unit) != INDEX_NONE) ||
		(PB->GetFieldUnitGridPoint(Unit) != FIntPoint::NoneValue);

	if (!bOnMyBoard)
	{
		Client_ShopRequestFinished();
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
	if (!GS)
	{
		Client_ShopRequestFinished();
		return;
	}

	auto PS = GetPlayerState<APCPlayerState>();
	if (!PS)
	{
		Client_ShopRequestFinished();
		return;
	}

	auto ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		Client_ShopRequestFinished();
		return;
	}
	
	APCPlayerBoard* PB = GetPlayerBoard();
	if (!IsValid(PB))
	{
		Client_ShopRequestFinished();
		return;
	}

	int32 RequiredCount = 0;

	// 벤치가 꽉 찼을 때
	if (PB->GetFirstEmptyBenchIndex() == INDEX_NONE)
	{
		auto SameSlotIndices = GetSameShopSlotIndices(SlotIndex);
		RequiredCount = GS->GetShopManager()->GetRequiredCountWithFullBench(PS, PS->GetShopSlots()[SlotIndex].UnitTag, SameSlotIndices.Num() + 1);
		
		if (auto AttributeSet = PS->GetAttributeSet())
		{
			// 모두 구매가 가능한지 골드 확인
			if (RequiredCount == 0 || AttributeSet->GetPlayerGold() < PS->GetShopSlots()[SlotIndex].UnitCost * RequiredCount)
			{
				Client_ShopRequestFinished();
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
				Client_ShopRequestFinished();
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

// void APCCombatPlayerController::LoadInventoryWidget()
// {
// 	if (IsLocalController())
// 	{
// 		if (!InventoryWidgetClass) return;
// 		
// 		InventoryWidget = CreateWidget<UPCPlayerInventoryWidget>(this, InventoryWidgetClass);
// 		if (!InventoryWidget) return;
//
// 		if (APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>())
// 		{
// 			InventoryWidget->BindToPlayerState(PCPlayerState);
// 			InventoryWidget->AddToViewport(8000);
// 		}
// 		else
// 		{
// 			// 안전책: 월드 틱 이후 GameState를 다시 확인
// 			GetWorldTimerManager().SetTimerForNextTick([this]()
// 			{
// 				if (APCPlayerState* PCPS2 = GetPlayerState<APCPlayerState>())
// 				{
// 					InventoryWidget->BindToPlayerState(PCPS2);
// 					InventoryWidget->AddToViewport(8000);
// 				}
// 			});
// 		}
// 		
// 		
// 	}
// }

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

	// 이미 있으면 보장만
	if (!IsValid(PlayerMainWidget))
	{
		PlayerMainWidget = CreateWidget<UPCPlayerMainWidget>(this, PlayerMainWidgetClass);
		if (!PlayerMainWidget) { UE_LOG(LogTemp, Warning, TEXT("CreateWidget failed")); return; }

		// 뷰포트에 항상 붙여둔다 (단 1회)
		PlayerMainWidget->AddToViewport();

		if (APCCombatGameState* PCCombatGameState = GetWorld()->GetGameState<APCCombatGameState>())
		{
			PlayerMainWidget->InitAndBind(PCCombatGameState);
			ShopWidget = PlayerMainWidget->GetShopWidget();
		}
		
		
		// UMG Construct 타이밍 대비 다음 프레임 보정 (옵션)
		FTimerHandle Th;
		GetWorld()->GetTimerManager().SetTimer(Th, [this]()
		{
			if (IsValid(PlayerMainWidget))
			{
				if (APCCombatGameState* PCCombatGameState = GetWorld()->GetGameState<APCCombatGameState>())
				{
					PlayerMainWidget->InitAndBind(PCCombatGameState);
					ShopWidget = PlayerMainWidget->GetShopWidget();
				}
			}
		}, 0.f, false);
	}
	else
	{
		// 재보장: 뷰포트에 없으면 붙이고, 바인딩 최신화
		if (!PlayerMainWidget->IsInViewport())
			PlayerMainWidget->AddToViewport();
		if (APCCombatGameState* PCCombatGameState = GetWorld()->GetGameState<APCCombatGameState>())
		{
			PlayerMainWidget->InitAndBind(PCCombatGameState);
			ShopWidget = PlayerMainWidget->GetShopWidget();
		}
	}
}

void APCCombatPlayerController::ShowWidget()
{
	if (!IsLocalController() || !IsValid(PlayerMainWidget))
		return;

	PlayerMainWidget->SetShopWidgetVisible(true);
	
	if (ShopWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		return;
	}
	ShopWidget->SetVisibility(ESlateVisibility::Visible);
}

void APCCombatPlayerController::HideWidget()
{
	if (!IsLocalController() || !IsValid(PlayerMainWidget))
		return;

	PlayerMainWidget->SetShopWidgetVisible(false);
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
	const bool bInBattle = GS && IsBattleTag(GS->GetGameStateTag());

	APCPlayerBoard* PB = GetPlayerBoard();
	if (!IsValid(PB))
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	bool  bIsField = false;
	int32 Y = INDEX_NONE, X = INDEX_NONE, BenchIdx = INDEX_NONE; // BenchIdx = 로컬
	FVector Snap = World;

	// 유닛이 있는 타일만 허용(bRequireUnit=true)
	if (!PB->WorldAnyTile(World, /*bPreferField*/ true, bIsField, Y, X, BenchIdx, Snap, 0.f, 0.f, /*bRequireUnit*/ false))
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	// 전투 중엔 필드에서 픽업 불가 (벤치만)
	if (bInBattle && bIsField)
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = bIsField ? PB->GetFieldUnit(Y, X) : PB->GetBenchUnit(BenchIdx);
	if (!IsValid(Unit) || !CanControlUnit(Unit))
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	// 벤치면 로컬 범위 한번 더 안전 확인
	if (!bIsField && (BenchIdx < 0 || BenchIdx >= PB->BenchSize))
	{
		Client_DragConfirm(false, DragId, World, nullptr);
		return;
	}

	CurrentDragId   = DragId;
	CurrentDragUnit = Unit;

	if (APCHeroUnitCharacter* PreviewUnit = Cast<APCHeroUnitCharacter>(Unit))
	{		
		Client_DragConfirm(true, DragId, Snap, PreviewUnit);
		Client_CurrentDragUnit(Unit);
	}
	else
	{
		Client_DragConfirm(true, DragId, Snap, nullptr);
	}
}

void APCCombatPlayerController::Server_EndDrag_Implementation(FVector World, int32 DragId)
{
	APCCombatGameState* GS = GetWorld()->GetGameState<APCCombatGameState>();
    const bool bInBattle = GS && IsBattleTag(GS->GetGameStateTag());

    // 드래그 유효성
    if (DragId != CurrentDragId || !CurrentDragUnit.IsValid())
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        return;
    }

    APCPlayerBoard* PB = GetPlayerBoard();
	
    if (!IsValid(PB))
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        return;
    }

    // 목표 타일 스냅 (전투중이면 벤치 우선)
    bool    bDstField  = false;
    int32   Y          = INDEX_NONE;
    int32   X          = INDEX_NONE;
    int32   BenchIdx   = INDEX_NONE;     // 로컬 벤치 인덱스
    FVector Snap       = World;
    const bool bPreferField = !bInBattle;

    if (!PB->WorldAnyTile(World, bPreferField, bDstField, Y, X, BenchIdx, Snap, 0.f, 0.f, /*bRequireUnit*/ false))
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        return;
    }

    // 전투 중엔 필드 배치 금지
    if (bInBattle && bDstField)
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        CurrentDragUnit = nullptr;
        CurrentDragId   = 0;
        return;
    }

    // 벤치면 로컬 범위 확인 (PlayerBoard는 적/아군 구분 없이 자기 벤치만 있음)
    if (!bDstField && (BenchIdx < 0 || BenchIdx >= PB->BenchSize))
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        CurrentDragUnit = nullptr;
        CurrentDragId   = 0;
        return;
    }

    APCBaseUnitCharacter* Unit = CurrentDragUnit.Get();
    if (!IsValid(Unit))
    {
        Client_DragEndResult(false, World, DragId, nullptr);
        return;
    }

    // 출발지 정보
    const FIntPoint SrcGrid  = PB->GetFieldUnitGridPoint(Unit);
    const bool      bSrcField= (SrcGrid != FIntPoint::NoneValue);
    const int32     SrcBench = bSrcField ? INDEX_NONE : PB->GetBenchUnitIndex(Unit);

    // 목적지 점유자
    APCBaseUnitCharacter* DstUnit = bDstField ? PB->GetFieldUnit(Y, X) : PB->GetBenchUnit(BenchIdx);
    if (DstUnit && !CanControlUnit(DstUnit))
    {
        Client_DragEndResult(false, Snap, DragId, nullptr);
        return;
    }

    // ───────────────────────────────────────────────────────────
    // 1) 빈 칸(또는 자기 자신)으로 이동
    // ───────────────────────────────────────────────────────────
    if (DstUnit == nullptr || DstUnit == Unit)
    {
    	
    	if (bSrcField)
    	{
    		PB->RemoveFromField(SrcGrid.X, SrcGrid.Y);
    	}
    	else if (SrcBench != INDEX_NONE)
    	{
    		PB->RemoveFromBench(SrcBench);
    	}

    	bool bPlaced;

    	if (bDstField)
    	{
    		if (!PB->PlaceUnitOnField(Y, X, Unit))
    		{
    			PB->PlaceUnitOnBench(SrcBench, Unit);
    			bPlaced = false;
    		}
		    else
		    {
		    	bPlaced = true;
		    }
    	}
	    else
	    {
	    	PB->PlaceUnitOnBench(BenchIdx, Unit);
	    	bPlaced = true;
	    }

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
        CurrentDragId   = 0;
        return;
    }

    // ───────────────────────────────────────────────────────────
    // 2) 스왑 (목적지에 내 유닛이 있을 때)
    // ───────────────────────────────────────────────────────────
    {
        // 유닛의 도착지 월드 위치 (스냅으로 충분하지만 bench/field 정확 좌표 쓰면 더 깔끔)
        const FVector UnitDest =
            bDstField ? PB->GetFieldWorldPos(Y, X)
                      : PB->GetBenchWorldPos(BenchIdx);

        // 스왑 수행
        const FIntPoint SrcGridForOther = PB->GetFieldUnitGridPoint(Unit);
        const int32     SrcBenchForOther= PB->GetBenchUnitIndex(Unit);

        if (PB->Swap(Unit, DstUnit))
        {
            // 상대 유닛의 목적지 = Unit의 원래 자리
            FVector OtherDest = FVector::ZeroVector;
            if (SrcGridForOther != FIntPoint::NoneValue)
                OtherDest = PB->GetFieldWorldPos(SrcGridForOther.X, SrcGridForOther.Y);
            else if (SrcBenchForOther != INDEX_NONE)
                OtherDest = PB->GetBenchWorldPos(SrcBenchForOther);

            // 비주얼 이동
            Multicast_LerpMove(Unit,    UnitDest,  LerpDuration);
            if (!OtherDest.IsNearlyZero())
                Multicast_LerpMove(DstUnit, OtherDest, LerpDuration);

            Client_DragEndResult(true, UnitDest, DragId, Cast<APCHeroUnitCharacter>(Unit));
        }
        else
        {
            Client_DragEndResult(false, World, DragId, nullptr);
        }

        CurrentDragUnit = nullptr;
        CurrentDragId   = 0;
    }
}

void APCCombatPlayerController::Client_DragConfirm_Implementation(bool bOk, int32 DragId, FVector StartSnap, APCHeroUnitCharacter* PreviewHero)
{
	if (!IsLocalController())
		return;

	APCCombatGameState* GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS)
		return;
	
	const bool bInBattle = IsBattleTag(GS->GetGameStateTag());
	
	
	if (bOk && PreviewHero)
	{
		if (APCPlayerBoard* PlayerBoard = GetLocalPlayerBoard())
		{
			PlayerBoard->OnHISM(true,bInBattle);
		}
		if (ShopWidget)
		{
			ShopWidget->ShowSellBox();
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

	if (APCPlayerBoard* PlayerBoard = PC->GetPlayerBoard())
	{
		PlayerBoard->OnHISM(false,false);
	}

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

APCPlayerBoard* APCCombatPlayerController::GetPlayerBoard() const
{
	if (const APCPlayerState* PCPlayerState = GetPlayerState<APCPlayerState>())
		return PCPlayerState->PlayerBoard;
	return nullptr;
}

APCPlayerBoard* APCCombatPlayerController::GetLocalPlayerBoard() const
{
	if (auto* PS = GetPlayerState<APCPlayerState>())
	{
		if (PS->PlayerBoard) return PS->PlayerBoard;
			// 아직 null이면 좌석으로 즉시 재검색
		PS->ResolvePlayerBoardOnClient();
		return PS->PlayerBoard;
	}
		return nullptr;
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

	FHitResult HitUnit;
	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Pawn), false, HitUnit))
	{
		if (APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(HitUnit.GetActor()))
		{
			Client_CheckHeroStatus(Unit);
			return;
		}
	}

	Client_CheckHeroStatus(nullptr);
}

void APCCombatPlayerController::Server_QueryHoverFromWorld_Implementation(const FVector& World)
{
	if (World.IsNearlyZero())
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	APCCombatGameState* GS = GetWorld()->GetGameState<APCCombatGameState>();
	const bool bInBattle = GS && IsBattleTag(GS->GetGameStateTag());

	APCPlayerBoard* PB = GetPlayerBoard();
	if (!IsValid(PB))
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	bool bField=false; int32 Y=-1, X=-1, BenchIdx=-1; FVector Snap=World;
	const bool bPreferField = !bInBattle; // ✅ 전투 중엔 벤치 우선
	if (!PB->WorldAnyTile(World, bPreferField, bField, Y, X, BenchIdx, Snap))
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	if (bInBattle && bField)
	{
		Client_TileHoverUnit(nullptr);		
		return;
	}

	APCBaseUnitCharacter* Unit = bField ? PB->GetFieldUnit(Y, X) : PB->GetBenchUnit(BenchIdx);
	Client_TileHoverUnit(Unit);
}

void APCCombatPlayerController::Server_QueryTileUnit_Implementation(bool bIsField, int32 Y, int32 X, int32 BenchIdx)
{
	APCPlayerBoard* PB = GetPlayerBoard();
	if (!IsValid(PB))
	{
		Client_TileHoverUnit(nullptr);
		return;
	}

	APCBaseUnitCharacter* Unit = bIsField ? PB->GetFieldUnit(Y, X) : PB->GetBenchUnit(BenchIdx);
	Client_TileHoverUnit(Unit);
	
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

void APCCombatPlayerController::Client_CheckHeroStatus_Implementation(APCBaseUnitCharacter* Unit)
{
	if (!IsLocalController())
		return;

	if (Unit != nullptr)
	{
		CachedCheckStatusUnit = Unit;
	}
	else
	{
		CachedCheckStatusUnit = nullptr;
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
	
}

void APCCombatPlayerController::Client_LoadGameResultWidget_Implementation(int32 Ranking)
{
	if (!GameResultWidgetClass) return;
		
	GameResultWidget = CreateWidget<UPCGameResultWidget>(this, GameResultWidgetClass);
	if (!GameResultWidget) return;

	GameResultWidget->SetRanking(Ranking);
	GameResultWidget->OpenMenu();
}