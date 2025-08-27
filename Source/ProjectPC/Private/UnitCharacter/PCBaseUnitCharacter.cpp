// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCBaseUnitCharacter.h"

#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Animation/Unit/PCUnitAnimInstance.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
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
	return Cast<UPCUnitAttributeSet>(GetUnitAbilitySystemComponent()->GetAttributeSet(UPCUnitAttributeSet::StaticClass()));
	//return GetAbilitySystemComponent()->GetSet<UPCUnitAttributeSet>();
}

const UPCDataAsset_BaseUnitData* APCBaseUnitCharacter::GetUnitDataAsset() const
{
	return nullptr;
}

FGameplayTag APCBaseUnitCharacter::GetUnitTypeTag() const
{
	return FGameplayTag::EmptyTag;
}

void APCBaseUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitAbilitySystem();
	SetAnimSetData();
}

void APCBaseUnitCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//InitAbilitySystem();
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

void APCBaseUnitCharacter::SetAnimSetData() const
{
	const UPCDataAsset_BaseUnitData* UnitData = GetUnitDataAsset();
	if (UPCDataAsset_UnitAnimSet* UnitAnimSet = UnitData ? UnitData->GetAnimData() : nullptr)
	{
		if (UPCUnitAnimInstance* UnitAnimInstance =	Cast<UPCUnitAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UnitAnimInstance->SetAnimSet(UnitAnimSet);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitAnimSet Nullptr"));
	}
}
