// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "GameFramework/GameInstanceSubsystem/PCUnitGERegistrySubsystem.h"

void UPCUnitAbilitySystemComponent::InitGAS()
{
	// GE & GA 적용은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	ApplyInitStatSet();
	GrantStartupAbilities();
}

void UPCUnitAbilitySystemComponent::ApplyInitStatSet()
{
	// GE 적용은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// 이미 Init GE를 적용했으면 바로 return (중복 적용 방지)
	if (InitStatSetGEHandle.IsValid())
	{
		return;
	}
	
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;
	
	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();

	
	// 유닛 기본 스탯을 부여하기 전에 유닛의 Type 태그를 먼저 부여함 (유닛 Type에 따라 부여하는 속성이 다르기 때문)
	if (!UnitData || !TryGrantUnitTypeTag())
		return;
	
	int32 UnitLevel = UnitCharacter->HasLevelSystem() ? UnitCharacter->GetUnitLevel() : 1;
	
	TMap<FGameplayTag, float> StatMap;
	UnitData->FillInitStatMap(UnitLevel, StatMap);
	
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	UPCUnitGERegistrySubsystem* UnitGERegistrySubsystem = GI ? GI->GetSubsystem<UPCUnitGERegistrySubsystem>() : nullptr;
	if (!UnitGERegistrySubsystem)
		return;

	const FGameplayTag InitGEClassTag = GameplayEffectTags::GE_Class_InitDefaultStat;
	if (const TSubclassOf<UGameplayEffect> InitGEClass = UnitGERegistrySubsystem->GetGEClass(InitGEClassTag))
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(InitGEClass, UnitLevel, MakeEffectContext());

		if (!SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("ApplyInitStatSetForLevel: SpecHandle invalid."));
			return;
		}
		
		for (const auto& Pair : StatMap)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}

		InitStatSetGEHandle = ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UPCUnitAbilitySystemComponent::GrantStartupAbilities()
{
	// GA 능력 부여는 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;
	
	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	if (!UnitData)
		return;
	
	TArray<TSubclassOf<UGameplayAbility>> GrantAbilities;
	UnitData->FillStartupAbilities(GrantAbilities);

	for (const auto& GAClass : GrantAbilities)
	{
		if (!*GAClass || FindAbilitySpecFromClass(GAClass))
			continue;

		GiveAbility(FGameplayAbilitySpec(GAClass, 1, INDEX_NONE, GetOwner()));
	}
}

bool UPCUnitAbilitySystemComponent::TryGrantUnitTypeTag()
{
	// GE 적용은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return false;

	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return false;
	
	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	if (!UnitData)
		return false;
	
	// 태그 중복 부여 방지
	if (HasMatchingGameplayTag(UnitCharacter->GetUnitTypeTag()))
		return true;

	const UGameInstance* GI = GetWorld()->GetGameInstance();
	UPCUnitGERegistrySubsystem* UnitGERegistrySubSystem = GI->GetSubsystem<UPCUnitGERegistrySubsystem>();
	if (const UGameplayEffect* UnitTypeGE = UnitGERegistrySubSystem->GetGrantUnitTypeGE_CDO(UnitCharacter->GetUnitTypeTag()))
	{
		ApplyGameplayEffectToSelf(
			UnitTypeGE,
			1.f,
			MakeEffectContext());

		return true;
	}

	// 만일 GE가 할당되지 않았을 경우에도 태그가 부여되도록 예외처리
	AddLooseGameplayTag(UnitCharacter->GetUnitTypeTag(), 1);
	return true;
}
