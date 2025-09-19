// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


APCPlayerState::APCPlayerState()
{
	bReplicates = true;
	PlayerAbilitySystemComponent = CreateDefaultSubobject<UPCPlayerAbilitySystemComponent>("PlayerAbilitySystemComponent");
	PlayerAbilitySystemComponent->AddAttributeSetSubobject(CreateDefaultSubobject<UPCPlayerAttributeSet>(TEXT("PlayerAttributeSet")));
	PlayerAttributeSet = PlayerAbilitySystemComponent->GetSet<UPCPlayerAttributeSet>();
	
	PlayerAbilitySystemComponent->SetIsReplicated(true);
	PlayerAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 60.f;
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

const UPCPlayerAttributeSet* APCPlayerState::GetAttributeSet() const
{
	return PlayerAttributeSet;
}

void APCPlayerState::OnRep_ShopSlots()
{
	OnShopSlotsUpdated.Broadcast();
}

void APCPlayerState::Client_ForceShopSlotsUpdate_Implementation()
{
	OnRep_ShopSlots();
}

void APCPlayerState::SetShopSlots(const TArray<FPCShopUnitData>& NewSlots)
{
	if (ShopSlots == NewSlots)
		Client_ForceShopSlotsUpdate();
	else
		ShopSlots = NewSlots;

	if (HasAuthority())
		OnRep_ShopSlots();
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
