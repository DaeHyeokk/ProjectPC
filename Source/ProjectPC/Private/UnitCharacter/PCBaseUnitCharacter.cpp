// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCBaseUnitCharacter.h"

#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"


APCBaseUnitCharacter::APCBaseUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 네트워크 설정
	//NetUpdateFrequency = 100.f;
	//MinNetUpdateFrequency = 66.f;
	
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,640.f, 0.f);
	GetCharacterMovement()->SetIsReplicated(true);

	bReplicates = true;
	SetReplicates(true);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.0f), FRotator(0.f,-90.f,0.f));
	GetMesh()->SetIsReplicated(true);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages;
}

UAbilitySystemComponent* APCBaseUnitCharacter::GetAbilitySystemComponent() const
{
	return GetUnitAbilitySystemComponent();
}

UPCUnitAbilitySystemComponent* APCBaseUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return nullptr;
}

const UPCUnitAttributeSet* APCBaseUnitCharacter::GetUnitAttributeSet() const
{
	//return Cast<UPCUnitAttributeSet>(UnitAbilitySystemComponent->GetAttributeSet(UPCUnitAttributeSet::StaticClass()));
	//return GetAbilitySystemComponent()->GetSet<UPCUnitAttributeSet>();
	return nullptr;
}

const UPCDataAsset_BaseUnitData* APCBaseUnitCharacter::GetUnitDataAsset() const
{
	return BaseUnitDataAsset;
}

FGameplayTag APCBaseUnitCharacter::GetUnitTypeTag() const
{
	return FGameplayTag::EmptyTag;
}

void APCBaseUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitAbilitySystem();
}

void APCBaseUnitCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void APCBaseUnitCharacter::InitAbilitySystem()
{
	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			GetUnitAbilitySystemComponent()->InitGAS();
		}
	}
}
