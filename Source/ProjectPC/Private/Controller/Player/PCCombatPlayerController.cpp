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

void APCCombatPlayerController::ShopRequest_BuyUnit(int32 SlotIndex)
{
	if (IsLocalController())
	{
		Server_BuyUnit(SlotIndex);
	}
}

void APCCombatPlayerController::ShopRequest_SellUnit()
{
	if (IsLocalController())
	{
		Server_SellUnit();
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

void APCCombatPlayerController::Server_BuyUnit_Implementation(int32 SlotIndex)
{
	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_BuyUnit;
			FGameplayEventData EventData;
			EventData.Instigator = PS;
			EventData.Target = PS;
			EventData.EventTag = GA_Tag;
			EventData.EventMagnitude = static_cast<float>(SlotIndex);

			ASC->HandleGameplayEvent(GA_Tag, &EventData);
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
			}
		}
	}
}

void APCCombatPlayerController::ClientCameraSetCarousel_Implementation(APCCarouselRing* CarouselRing, int32 SeatIndex, float BlendTime)
{
	if (!IsLocalController() || !IsValid(CarouselRing))
		return;
	//CarouselRing->ApplyCentralViewForSeat(this, CurrentFocusedSeatIndex, 0.f);
	//FadeSwitchCamera(CarouselRing, 0.1f,0.05f,0,0.1f, false);
	SetViewTargetWithBlend(CarouselRing,0);
	
}

void APCCombatPlayerController::SetOverlappedUnit(APCHeroUnitCharacter* NewUnit)
{
	OverlappedUnit = NewUnit;
	UE_LOG(LogTemp, Warning, TEXT("Controller Hero Overlap"));
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
		// PlayerMainWidget->AddToViewport();
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
			// PlayerMainWidget->AddToViewport();
		PlayerMainWidget->InitAndBind();
		
	}

	if (!IsValid(ShopWidget))
	{
		ShopWidget = CreateWidget<UPCShopWidget>(this, ShopWidgetClass);
		if (!ShopWidget)
			return;
		// ShopWidget->AddToViewport();
	}
	else
	{
		if (!ShopWidget->IsInViewport())
		{
			// ShopWidget->AddToViewport();
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
		Client_DragConfirm(false, DragId, World);
		return;
	}

	bool bIsField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;

	if (!TM->WorldAnyTile(World, true, bIsField, Y, X, BenchIdx, Snap))
	{
		Client_DragConfirm(false,DragId, World);
		return;
	}

	APCBaseUnitCharacter* Unit = bIsField ? TM->GetFieldUnit(Y,X) : TM->GetBenchUnit(BenchIdx);

	if (!Unit || !CanControlUnit(Unit))
	{
		Client_DragConfirm(false, DragId, World);
		return;
	}

	CurrentDragId = DragId;
	CurrentDragUnit = Unit;

	Client_DragConfirm(true, DragId, Snap);
	
}

void APCCombatPlayerController::Server_UpdateDrag_Implementation(FVector World, int32 DragId)
{
	if (DragId != CurrentDragId || !CurrentDragUnit.IsValid())
		return;

	UPCTileManager* TM = GetTileManager();
	if (!TM)
		return;
	bool bField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;

	if (!TM->WorldAnyTile(World,true, bField, Y, X, BenchIdx, Snap))
		return;

	APCBaseUnitCharacter* Unit = CurrentDragUnit.Get();
	bool bValid = false;
	
	if (bField)
	{
		const bool bOk = IsAllowFieldY(Y);
		if (bOk)
		{
			bValid = TM->IsInRange(Y,X) && (TM->CanUse(Y,X,Unit) || TM->GetFieldUnit(Y,X) == Unit);
		}
	}
	else
	{
		APCBaseUnitCharacter* BenchUnit = TM->GetBenchUnit(BenchIdx);
		bValid = (BenchUnit == nullptr || BenchUnit == Unit);
	}

	Client_DragHint(Snap, bValid, DragId);
}

void APCCombatPlayerController::Server_EndDrag_Implementation(FVector World, int32 DragId)
{
	if (DragId != CurrentDragId || !CurrentDragUnit.IsValid())
	{
		Client_DragEndResult(false, World, DragId);
		return;
	}

	UPCTileManager* TM = GetTileManager();
	if (!TM)
	{
		Client_DragEndResult(false, World, DragId);
		return;
	}

	bool bField = false;
	int32 Y = -1;
	int32 X = -1;
	int32 BenchIdx = -1;
	FVector Snap = World;
	if (!TM->WorldAnyTile(World, true, bField, Y, X, BenchIdx, Snap))
	{
		Client_DragEndResult(false, World, DragId);
		return;
	}

	APCBaseUnitCharacter* Unit = CurrentDragUnit.Get();

	if (bField && !IsAllowFieldY(Y))
	{
		Client_DragEndResult(false, Snap, DragId);
		return;
	}
	RemoveFromCurrentSlot(TM,Unit);

	bool bPlaced = false;
	if (bField)
	{
		if (TM->IsInRange(Y,X) && TM->CanUse(Y,X,Unit) || TM->GetFieldUnit(Y,X) == Unit)
		{
			bPlaced = TM->PlaceUnitOnField(Y,X,Unit);
		}
	}
	else
	{
		APCBaseUnitCharacter* BenchUnit = TM->GetBenchUnit(BenchIdx);
		if (BenchUnit == nullptr || BenchUnit == Unit)
		{
			bPlaced = TM->PlaceUnitOnBench(BenchIdx,Unit);
		}
	}

	Client_DragEndResult(bPlaced, bPlaced ? Snap : World, DragId);
	CurrentDragUnit = nullptr;
	CurrentDragId = 0;
}

void APCCombatPlayerController::Client_DragConfirm_Implementation(bool bOk, int32 DragId, FVector StartSnap)
{
	if (!IsLocalController())
		return;

	if (const APCCombatBoard* Board = FindBoardBySeatIndex(HomeBoardSeatIndex))
	{
		Board -> OnHism(true);
	}
	
	if (DragComponent)
	{
		DragComponent->OnServerDragConfirm(bOk, DragId, StartSnap);
	}
}

void APCCombatPlayerController::Client_DragHint_Implementation(FVector Snap, bool bValid, int32 DragId)
{
	if (!IsLocalController())
		return;
	
	if (DragComponent)
	{
		DragComponent->OnServerDragHint(Snap, bValid, DragId);
	}
}

void APCCombatPlayerController::Client_DragEndResult_Implementation(bool bSuccess, FVector FinalSnap, int32 DragId)
{
	if (!IsLocalController())
		return;
	
	if (const APCCombatBoard* Board = FindBoardBySeatIndex(HomeBoardSeatIndex))
	{
		Board -> OnHism(false);
	}
	
	if (DragComponent)
	{
		DragComponent->OnServerDragEndResult(bSuccess, FinalSnap, DragId);
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
