// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PCPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Item/PCPlayerInventory.h"
#include "UI/PlayerMainWidget/PCPlayerOverheadWidget.h"


APCPlayerCharacter::APCPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	GetCapsuleComponent()->InitCapsuleSize(60.f, 60.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	bIsDead = false;
	
	OverHeadWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidgetComp->SetupAttachment(RootComponent);
	OverHeadWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	OverHeadWidgetComp->SetPivot(FVector2D(0.5f, 1.0f));

	// 캐러샐 유닛 캐리 슬롯
	CarrySlot = CreateDefaultSubobject<USceneComponent>(TEXT("CarrySlot"));
	CarrySlot->SetupAttachment(GetRootComponent());
	CarrySlot->SetRelativeLocation(FVector( -120.f, 0.f, 20.f));
	CarrySlot->SetRelativeRotation(FRotator(0.f,0.f,0.f));

	NetUpdateFrequency = 60.f;
}

void APCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);
}

void APCPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (auto PS = GetPlayerState<APCPlayerState>())
	{
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, this);
			if (!CharacterTags.IsEmpty())
			{
				ASC->AddLooseGameplayTags(CharacterTags);	
			}
		}
	}
}

void APCPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (APCPlayerState* PS = GetPlayerState<APCPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, this);
		}

		if (OverHeadWidgetComp)
		{
			if (auto OverheadWidget = Cast<UPCPlayerOverheadWidget>(OverHeadWidgetComp->GetUserWidgetObject()))
			{
				OverheadWidget->BindToPlayerState(PS);
			}
		}
	}
}

void APCPlayerCharacter::Server_RequestCarouselPick_Implementation()
{
	for (TActorIterator<APCCarouselRing> It(GetWorld()); It; ++It)
	{
		CarouselRing = *It;
	}
	CarouselRing->Server_TryPickForPlayer(this);
}

void APCPlayerCharacter::CarouselUnitToSpawn()
{
	APCPlayerState* PS = GetPlayerState<APCPlayerState>();
	if (!PS) return;
	
	UPCPlayerInventory* PlayerInventory = PS->GetPlayerInventory();
	if (!PlayerInventory) return;

	const FGameplayTag UnitTag = CarouselUnitData.UnitTag;
	const FGameplayTag ItemTag = CarouselUnitData.ItemTag;
	
	PS->UnitSpawn(UnitTag);
	PlayerInventory->AddItemToInventory(ItemTag);
}

void APCPlayerCharacter::PlayerDie()
{
	GetCharacterMovement()->StopMovementImmediately();
	
	auto PC = Cast<APCCombatPlayerController>(GetController());
	if (!PC) return;
	
	if (HasAuthority())
	{
		bIsDead = true;

		PC->Client_HideWidget();
	}
	
	DisableInput(PC);
}

void APCPlayerCharacter::OnPlayerDeathAnimFinished()
{
	Destroy();
}

void APCPlayerCharacter::Client_PlayMontage_Implementation(UAnimMontage* Montage, float InPlayRate)
{
	if (!Montage) return;

	if (auto CharMesh = GetMesh())
	{
		if (auto Anim = CharMesh->GetAnimInstance())
		{
			Anim->Montage_Play(Montage, InPlayRate); 
		}
	}
}

void APCPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCPlayerCharacter, bIsDead);
}

void APCPlayerCharacter::SetOverHeadWidgetPosition(FGameplayTag PlayerStateTag)
{
	if (PlayerStateTag == PlayerGameplayTags::Player_State_Normal)
	{
		OverHeadWidgetComp->SetPivot(FVector2D(0.5f, 1.0f));
	}
	else if (PlayerStateTag == PlayerGameplayTags::Player_State_Carousel)
	{
		OverHeadWidgetComp->SetPivot(FVector2D(0.5f, 2.2f));
	}
}

void APCPlayerCharacter::SetOverHeadWidget()
{
	if (APCPlayerState* PS = GetPlayerState<APCPlayerState>())
	{	
		if (OverHeadWidgetComp)
		{
			if (auto OverheadWidget = Cast<UPCPlayerOverheadWidget>(OverHeadWidgetComp->GetUserWidgetObject()))
			{
				OverheadWidget->BindToPlayerState(PS);
			}
		}
	}
}


