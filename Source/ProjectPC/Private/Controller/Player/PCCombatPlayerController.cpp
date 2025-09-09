// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/PCCombatPlayerController.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "BaseGameplayTags.h"
#include "Shop/PCShopManager.h"
#include "UI/Shop/PCShopWidget.h"
#include "DataAsset/Player/PCDataAsset_PlayerInput.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


APCCombatPlayerController::APCCombatPlayerController()
{
	bShowMouseCursor = true;
	bAutoManageActiveCameraTarget = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
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
	// ShopRequest_SellUnit()
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

void APCCombatPlayerController::ShopRequest_SellUnit(FGameplayTag UnitTag)
{
	if (IsLocalController())
	{
		Server_SellUnit(UnitTag);
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

void APCCombatPlayerController::Server_SellUnit_Implementation(FGameplayTag UnitTag)
{
	// if (auto PS = GetPlayerState<APCPlayerState>())
	// {
	// 	if (auto ASC = PS->GetAbilitySystemComponent())
	// 	{
	// 		FGameplayTag GA_Tag = PlayerGameplayTags::Player_GA_Shop_SellUnit;
	// 		FGameplayEventData EventData;
	// 		EventData.Instigator = PS;
	// 		EventData.Target = PS;
	// 		EventData.EventTag = GA_Tag;
	//
	// 		ASC->HandleGameplayEvent(GA_Tag, &EventData);
	// 	}
	// }
}

void APCCombatPlayerController::ClientCameraSet_Implementation(int32 BoardIndex, float BlendTime)
{
	TArray<AActor*> Boards;
	UGameplayStatics::GetAllActorsOfClass(this, APCCombatBoard::StaticClass(), Boards);
	for (AActor* Actor : Boards)
	{
		if (auto* CombatBoards = Cast<APCCombatBoard>(Actor))
		{
			if (CombatBoards->BoardSeatIndex == BoardIndex)
			{
				CombatBoards->ApplyLocalBottomView(this, BoardIndex, BlendTime);
				return;
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

void APCCombatPlayerController::ClientStageChanged_Implementation(EPCStageType NewStage, const FString& StageRoundName,
	float Seconds)
{
	
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

void APCCombatPlayerController::ClientSetHomeBoardIndex_Implementation(int32 InHomeBoardIdx)
{
	HomeBoardSeatIndex = InHomeBoardIdx;
}

void APCCombatPlayerController::ClientFocusBoardBySeatIndex_Implementation(int32 BoardSeatIndex,
	bool bRespectFlipPolicy, float Blend)
{
	APCCombatBoard* CombatBoard = FindBoardBySeatIndex(BoardSeatIndex);
	if (!CombatBoard) return;

	const bool bFlip = bRespectFlipPolicy ? ShouldFlipForBoardIndex(BoardSeatIndex) : false;
	CombatBoard->ApplyBattleCamera(this,bFlip,Blend);
}

bool APCCombatPlayerController::ShouldFlipForBoardIndex(int32 BoardSeatIndex) const
{
	return (HomeBoardSeatIndex != INDEX_NONE && BoardSeatIndex != HomeBoardSeatIndex);
}
