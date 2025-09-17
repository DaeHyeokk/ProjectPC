// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCBaseUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Animation/Unit/PCUnitAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "DSP/BufferDiagnostics.h"
#include "EntitySystem/MovieSceneComponentDebug.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"


APCBaseUnitCharacter::APCBaseUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 네트워크 설정
	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 66.f;
	
	bReplicates = true;
	SetReplicates(true);
	
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->SetIsReplicated(false);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,640.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.0f), FRotator(0.f,-90.f,0.f));
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages;
	
	StatusBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusBarWidgetComponent"));
	StatusBarComp->SetIsReplicated(true);
	StatusBarComp->SetupAttachment(GetMesh());
	StatusBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	StatusBarComp->SetDrawSize({300.f, 100.f});
	StatusBarComp->SetPivot({0.5f, 1.f});
	StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));
}

UAbilitySystemComponent* APCBaseUnitCharacter::GetAbilitySystemComponent() const
{
	return GetUnitAbilitySystemComponent();
}

UPCUnitAbilitySystemComponent* APCBaseUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return nullptr;
}

UPCDataAsset_UnitAnimSet* APCBaseUnitCharacter::GetUnitAnimSetDataAsset() const
{
	return GetUnitDataAsset() ? GetUnitDataAsset()->GetAnimSetData() : nullptr;
}

const UPCDataAsset_UnitAbilityConfig* APCBaseUnitCharacter::GetUnitAbilityConfigDataAsset() const
{
	return GetUnitDataAsset() ? GetUnitDataAsset()->GetAbilityConfigData() : nullptr;
}

const UPCDataAsset_BaseUnitData* APCBaseUnitCharacter::GetUnitDataAsset() const
{
	return nullptr;
}

void APCBaseUnitCharacter::SetOutlineMID(UMaterialInterface* OutlineMat)
{
	if (!OutlineMat)
		return;

	if (!OutlineMID)
	{
		OutlineMID = UMaterialInstanceDynamic::Create(OutlineMat, this);
	}
}

void APCBaseUnitCharacter::SetOutlineEnabled(bool bEnable) const
{
	if (!GetMesh())
		return;

	if (bEnable)
	{
		if (OutlineMID)
		{
			GetMesh()->SetOverlayMaterial(OutlineMID);
		}
	}
	else
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}

FGameplayTag APCBaseUnitCharacter::GetUnitTypeTag() const
{
	return FGameplayTag::EmptyTag;
}

FGenericTeamId APCBaseUnitCharacter::GetGenericTeamId() const
{
	const uint8 Clamped = static_cast<uint8>(FMath::Clamp(TeamIndex, 0, 254));
	return FGenericTeamId(Clamped);
}

void APCBaseUnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilitySystem();
	SetAnimSetData();

	if (StatusBarClass)
	{
		StatusBarComp->SetWidgetClass(StatusBarClass);
		ReAttachStatusBarToSocket();
		
		if (UUserWidget* W = StatusBarComp->GetUserWidgetObject())
		{
			InitStatusBarWidget(W);
		}
	}
}

void APCBaseUnitCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void APCBaseUnitCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCBaseUnitCharacter, UnitTag);
	DOREPLIFETIME(APCBaseUnitCharacter, TeamIndex);
	DOREPLIFETIME(APCBaseUnitCharacter, bIsOnField);
}

void APCBaseUnitCharacter::InitStatusBarWidget(UUserWidget* StatusBarWidget)
{
	// 하위 클래스에서 오버라이드 해서 구현
}

void APCBaseUnitCharacter::ReAttachStatusBarToSocket() const
{
	if (USkeletalMeshComponent* SkeMesh = GetMesh())
	{
		if (SkeMesh->DoesSocketExist(StatusBarSocketName))
		{
			StatusBarComp->AttachToComponent(
				SkeMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				StatusBarSocketName);
			StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));
		}
	}
}

void APCBaseUnitCharacter::OnRep_UnitTag()
{
	// 유닛이 Spawn 되면 UnitTag 값이 세팅됨 -> 클라에서 OnRep_UnitTag 호출
	// -> 클라에서 보여줄 유닛 데이터 세팅 (메쉬, AnimBP, StatusBar UI)
	if (UWorld* W = GetWorld())
	{
		if (auto* SpawnSubSystem = W->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			const auto* Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			if (!Definition)
			{
				SpawnSubSystem->EnsureConfigFromGameState();
				Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			}
			if (Definition)
			{
				SpawnSubSystem->ApplyDefinitionDataVisuals(this, Definition);
			}
		}
	}
}

void APCBaseUnitCharacter::PushTeamIndexToController() const
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->SetGenericTeamId(GetGenericTeamId());
	}
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
	if (UPCDataAsset_UnitAnimSet* UnitAnimSet = UnitData ? UnitData->GetAnimSetData() : nullptr)
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

void APCBaseUnitCharacter::ChangedOnTile(const bool IsOnField)
{
	if (bIsOnField != IsOnField)
	{
		bIsOnField = IsOnField;
	}
}


// void APCBaseUnitCharacter::OnRep_IsOnField()
// {
// 	if (bIsOnField)
// 	{
// 		if (const UPCDataAsset_UnitAnimSet* UnitAnimSet = GetUnitAnimSetDataAsset())
// 		{
// 			if (UAnimMontage* Montage = UnitAnimSet->GetAnimMontageByTag(UnitGameplayTags::Unit_Montage_LevelStart))
// 			{
// 				if (Montage)
// 					GetMesh()->GetAnimInstance()->Montage_Play(Montage);
// 			}
// 		}
// 	}
// }