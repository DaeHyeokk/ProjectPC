// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCCarouselHeroCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Animation/Unit/PCCarouselHeroAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APCCarouselHeroCharacter::APCCarouselHeroCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetReplicates(true);

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	if (AbilitySystemComp)
	{
		AbilitySystemComp->SetIsReplicated(true);
		AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		UPCHeroUnitAttributeSet* HeroAttrSet = CreateDefaultSubobject<UPCHeroUnitAttributeSet>(TEXT("HeroUnitAttributeSet"));
		AbilitySystemComp->AddAttributeSetSubobject(HeroAttrSet);
		HeroAttributeSet = AbilitySystemComp->GetSet<UPCHeroUnitAttributeSet>();
	}

	StatusBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusBar"));
	StatusBarComp->SetupAttachment(GetMesh());
	StatusBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	StatusBarComp->SetDrawAtDesiredSize(true);
	StatusBarComp->SetPivot({0.5f, 1.f});
	StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.f), FRotator(0.f,-90.f,0.f));
}


void APCCarouselHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}
	
	InitCarouselAnimInstance();
	AttachStatusBarToSocket();
}

void APCCarouselHeroCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCCarouselHeroCharacter, UnitTag);
	DOREPLIFETIME(APCCarouselHeroCharacter, ItemTag);
}

void APCCarouselHeroCharacter::AttachStatusBarToSocket() const
{
	if (USkeletalMeshComponent* SkComp = GetMesh())
	{
		if (SkComp->DoesSocketExist(StatusBarSocketName))
		{
			StatusBarComp->AttachToComponent(
				SkComp,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				StatusBarSocketName);
			StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));
		}
	}
}

void APCCarouselHeroCharacter::InitAttributeSet()
{
	if (!AbilitySystemComp || !HeroAttributeSet || !HeroData)
		return;
	
	TMap<FGameplayAttribute, float> StatMap;
	HeroData->FillInitStatMap(1, StatMap);
	
	for (auto& KV : StatMap)
	{
		AbilitySystemComp->SetNumericAttributeBase(KV.Key, KV.Value);
	}

	SetNetDormancy(DORM_Awake);
	ForceNetUpdate();
}

void APCCarouselHeroCharacter::SetHeroUnitDataAsset(UPCDataAsset_HeroUnitData* InHeroData)
{
	if (InHeroData)
	{
		HeroData = InHeroData;
		
		if (HasAuthority())
		{
			InitAttributeSet();
		}
	}
}

void APCCarouselHeroCharacter::SetUnitTag(const FGameplayTag& InTag)
{
	if (HasAuthority())
	{
		UnitTag = InTag;

		if (GetNetMode() == NM_ListenServer)
			OnRep_UnitTag();
	}
}

TArray<FGameplayTag> APCCarouselHeroCharacter::GetEquipItemTags() const
{
	TArray<FGameplayTag> Result;
	if (ItemTag.IsValid())
		Result.Add(ItemTag);

	return Result;
}

void APCCarouselHeroCharacter::OnRep_UnitTag()
{
	if (UWorld* World = GetWorld())
	{
		if (auto* SpawnSubsystem = World->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			const UPCDataAsset_UnitDefinition* Definition = SpawnSubsystem->ResolveDefinition(UnitTag);
			if (!Definition)
			{
				SpawnSubsystem->EnsureConfigFromGameState();
				Definition = SpawnSubsystem->ResolveDefinition(UnitTag);
			}

			if (Definition)
			{
				SpawnSubsystem->ApplyDefinitionData(this, Definition);
				AttachStatusBarToSocket();
			}
		}
	}
}

void APCCarouselHeroCharacter::SetItemTag(const FGameplayTag& InItemTag)
{
	if (HasAuthority())
	{
		ItemTag = InItemTag;

		if (GetNetMode() == NM_ListenServer)
			OnRep_ItemTag();
	}
}

void APCCarouselHeroCharacter::OnRep_ItemTag()
{
	InitStatusBar();
}

void APCCarouselHeroCharacter::InitCarouselAnimInstance() const
{
	if (UPCDataAsset_UnitAnimSet* UnitAnimSet = HeroData ? HeroData->GetAnimSetData() : nullptr)
	{
		if (UPCCarouselHeroAnimInstance* CarouselAnimInstance =	Cast<UPCCarouselHeroAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			CarouselAnimInstance->InitAnimData(UnitAnimSet);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitAnimSet Nullptr"));
	}
}

void APCCarouselHeroCharacter::InitStatusBar()
{
	if (!StatusBarComp)
		return;

	if (UUserWidget* Widget = StatusBarComp->GetUserWidgetObject())
	{
		// 여기에서 캐러셀 유닛 상태 UI 초기화
	}
}
