// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PCPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"

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
	}
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
		// PC->Client_LoadGameResultWidget(Ranking);
		
		// 죽은 플레이어가 가지고 있던 유닛 상점 반환
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


