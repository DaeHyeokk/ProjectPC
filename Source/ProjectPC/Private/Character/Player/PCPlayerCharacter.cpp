// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PCPlayerCharacter.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameState/PCCombatGameState.h"

#include "Navigation/PathFollowingComponent.h"
#include "Net/UnrealNetwork.h"


APCPlayerCharacter::APCPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Move Test를 위한 카메라 세팅
	
	GetCapsuleComponent()->InitCapsuleSize(60.f, 60.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
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

	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 60.f;

	bIsDead = false;
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

		// 죽은 플레이어가 가지고 있던 유닛 상점 반환
	}

	if (IsLocallyControlled())
	{
		DisableInput(PC);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void APCPlayerCharacter::OnPlayerDeathAnimFinished()
{
	Destroy();
}

void APCPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCPlayerCharacter, bIsDead);
}


