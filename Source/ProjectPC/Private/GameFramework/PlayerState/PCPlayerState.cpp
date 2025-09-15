// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

APCPlayerState::APCPlayerState()
{
	bReplicates = true;
	PlayerAbilitySystemComponent = CreateDefaultSubobject<UPCPlayerAbilitySystemComponent>("PlayerAbilitySystemComponent");
	PlayerAttributeSet = CreateDefaultSubobject<UPCPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	PlayerAbilitySystemComponent->AddAttributeSetSubobject(PlayerAttributeSet);
}

void APCPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
		PlayerAbilitySystemComponent->ApplyInitializedAbilities();
		PlayerAbilitySystemComponent->ApplyInitializedEffects();
	}
	else
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* APCPlayerState::GetAbilitySystemComponent() const
{
	return PlayerAbilitySystemComponent; 
}

void APCPlayerState::OnRep_ShopSlots()
{
	OnShopSlotsUpdated.Broadcast();
}

void APCPlayerState::SetShopSlots(const TArray<FPCShopUnitData>& NewSlots)
{
	ShopSlots = NewSlots;
}

const TArray<FPCShopUnitData>& APCPlayerState::GetShopSlots()
{
	return ShopSlots;
}

void APCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCPlayerState, bIsReady);
	DOREPLIFETIME(APCPlayerState, LocalUserId);
	DOREPLIFETIME(APCPlayerState, bIsLeader);
	DOREPLIFETIME(APCPlayerState, SeatIndex);
	DOREPLIFETIME(APCPlayerState, bIdentified);
	DOREPLIFETIME(APCPlayerState, ShopSlots);
}
