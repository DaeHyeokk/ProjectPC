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
#include "DataAsset/Player/PCDataAsset_PlayerInput.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
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
	}
}

void APCCombatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	
	EnsureMainHUDCreated();
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

void APCCombatPlayerController::ClientCameraSetCarousel_Implementation(APCCarouselRing* CarouselRing, float BlendTime)
{
	if (APCCarouselRing* PCCarouselRing = Cast<APCCarouselRing>(CarouselRing))
	{
		PCCarouselRing->ApplyCentralView(this, BlendTime);
	}
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
		//PlayerMainWidget->AddToViewport(50);
		PlayerMainWidget->HideWidget();
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
			//PlayerMainWidget->AddToViewport(50);
		PlayerMainWidget->InitAndBind();
		PlayerMainWidget->HideWidget();
	}
}

void APCCombatPlayerController::ShowWidget(bool bAnimate)
{
	if (!IsLocalController() || !IsValid(PlayerMainWidget))
		return;
	PlayerMainWidget->ShowWidget();
	
}

void APCCombatPlayerController::HideWidget()
{
	if (!IsLocalController() || IsValid(PlayerMainWidget))
		return;
	PlayerMainWidget->HideWidget();
}

void APCCombatPlayerController::Client_ShowWidget_Implementation()
{
	ShowWidget(true);
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
		SetViewTargetWithBlend(CombatBoard, Blend);
		CurrentFocusedSeatIndex = BoardSeatIndex;
	}
}
