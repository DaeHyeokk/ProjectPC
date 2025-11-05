// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCBaseUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Animation/Unit/PCUnitAnimInstance.h"
#include "Component/PCUnitEquipmentComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Net/UnrealNetwork.h"


APCBaseUnitCharacter::APCBaseUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 네트워크 설정
	// NetUpdateFrequency = 100.f;
	// MinNetUpdateFrequency = 66.f;
	
	bReplicates = true;
	SetReplicates(true);
	
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->SetIsReplicated(false);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,640.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
	
	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.0f), FRotator(0.f,-90.f,0.f));
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	
	GetCapsuleComponent()->SetCapsuleSize(76.f, 100.f, true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	StatusBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusBarWidgetComponent"));
	StatusBarComp->SetIsReplicated(true);
	StatusBarComp->SetupAttachment(GetMesh());
	StatusBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	//StatusBarComp->SetDrawSize({300.f, 100.f});
	StatusBarComp->SetDrawAtDesiredSize(true);
	StatusBarComp->SetPivot({0.5f, 1.f});
	StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));

	EquipmentComp = CreateDefaultSubobject<UPCUnitEquipmentComponent>(TEXT("EquipmentComponent"));
	EquipmentComp->SetIsReplicated(true);
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

UPCDataAsset_ProjectileData* APCBaseUnitCharacter::GetUnitProjectileDataAsset() const
{
	return GetUnitDataAsset() ? GetUnitDataAsset()->GetProjectileData() : nullptr;
}

UPCDataAsset_BaseUnitData* APCBaseUnitCharacter::GetUnitDataAsset() const
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

FGenericTeamId APCBaseUnitCharacter::GetGenericTeamId() const
{
	const uint8 Clamped = static_cast<uint8>(FMath::Clamp(TeamIndex, 0, 254));
	return FGenericTeamId(Clamped);
}

TArray<FGameplayTag> APCBaseUnitCharacter::GetEquipItemTags() const
{
	TArray<FGameplayTag> Result;
	if (!EquipmentComp)
		return Result;

	for (const FGameplayTag& ItemTag : EquipmentComp->GetSlotItemTags())
	{
		if (!ItemTag.IsValid())
			break;
		
		Result.Add(ItemTag);
	}

	return Result;
}

void APCBaseUnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APCCombatGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr)
	{
		GameStateChangedHandle =
			GS->OnGameStateTagChanged.AddUObject(
				this, &ThisClass::OnGameStateChanged);
	
		OnGameStateChanged(GS->GetGameStateTag());
	}
	
	InitAbilitySystem();
	
	if (auto* ASC = GetAbilitySystemComponent())
	{
		DeadHandle = ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Dead)
		.AddUObject(this, &ThisClass::OnUnitStateTagChanged);

		StunHandle = ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Stun)
		.AddUObject(this, &ThisClass::OnUnitStateTagChanged);
	}
	
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

	if (UPCUnitAnimInstance* UnitAnimInstance = Cast<UPCUnitAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		UnitAnimInstance->PlayLevelStartMontage();
	}
}

void APCBaseUnitCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APCCombatGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr)
	{
		if (GameStateChangedHandle.IsValid())
		{
			GS->OnGameStateTagChanged.Remove(GameStateChangedHandle);
		}
	}

	if (DeadHandle.IsValid())
		GetAbilitySystemComponent()->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Dead).Remove(DeadHandle);
	if (StunHandle.IsValid())
		GetAbilitySystemComponent()->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Stun).Remove(StunHandle);
	
	Super::EndPlay(EndPlayReason);
}

void APCBaseUnitCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void APCBaseUnitCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APCBaseUnitCharacter, TeamIndex);
	DOREPLIFETIME(APCBaseUnitCharacter, bIsOnField);
	DOREPLIFETIME(APCBaseUnitCharacter, bIsCombatWin);
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


void APCBaseUnitCharacter::Die()
{
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			if (!bIsDead)
			{
				ASC->AddLooseGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead);
				ASC->AddReplicatedLooseGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead);

				ASC->CancelAllAbilities();
				OnUnitDied.Broadcast(this);
			}
		}
	}
}

void APCBaseUnitCharacter::OnDeathAnimCompleted()
{
	SetActorLocation(FVector(99999.f,99999.f,99999.f));
}

void APCBaseUnitCharacter::CombatWin(APCPlayerState* TargetPS)
{
	if (!HasAuthority() || !bIsOnField || bIsDead)
		return;

	SetActorRotation(FRotator(0.f,-180.f,0));
	bIsCombatWin = true;
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		APawn* TargetPawn = TargetPS ? TargetPS->GetPawn() : nullptr;
		
		FGameplayEventData EventData;
		EventData.EventTag = UnitGameplayTags::Unit_Event_Combat_Win;
		EventData.Instigator = this;
		EventData.Target = TargetPawn;

		ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
	}
}

void APCBaseUnitCharacter::CombatDraw(APCPlayerState* TargetPS)
{
	if (!HasAuthority() || !bIsOnField || bIsDead)
		return;
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		APawn* TargetPawn = TargetPS ? TargetPS->GetPawn() : nullptr;
		
		FGameplayEventData EventData;
		EventData.EventTag = UnitGameplayTags::Unit_Event_Combat_Win;
		EventData.Instigator = this;
		EventData.Target = TargetPawn;

		ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
	}
}

void APCBaseUnitCharacter::OnUnitStateTagChanged(FGameplayTag Tag, int32 NewCount)
{
	if (Tag.MatchesTagExact(UnitGameplayTags::Unit_State_Combat_Dead))
	{
		bIsDead = (NewCount > 0);
	}
	else if (Tag.MatchesTagExact(UnitGameplayTags::Unit_State_Combat_Stun))
	{
		bIsStunned = (NewCount > 0);
	}
}
