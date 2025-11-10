// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_BuyUnit.h"

#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "Shop/PCShopManager.h"


UPCGameplayAbility_BuyUnit::UPCGameplayAbility_BuyUnit()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_GA_Shop_BuyUnit);

	ActivationRequiredTags.AddTag(PlayerGameplayTags::Player_State_Normal);

	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Dead);
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Carousel);

	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_GA_Shop_BuyUnit;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UPCGameplayAbility_BuyUnit::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 서버 권위, CostGE 클래스가 유효하면 Activate
	if (ActorInfo && ActorInfo->IsNetAuthority() && CostGameplayEffectClass)
	{
		return true;
	}
	
	return false;
}

bool UPCGameplayAbility_BuyUnit::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}

void UPCGameplayAbility_BuyUnit::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpecHandle.IsValid())
	{
		if (BuyCount == 0)
		{
			// 1개 구매 시
			CostSpecHandle.Data->SetSetByCallerMagnitude(CostTag, -UnitCost);
		}
		else
		{
			// 여러개 동시 구매 시
			CostSpecHandle.Data->SetSetByCallerMagnitude(CostTag, -(UnitCost * BuyCount));
		}

		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
		}
	}
}

void UPCGameplayAbility_BuyUnit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData || !TriggerEventData->TargetData.IsValid(0))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	auto PS = Cast<APCPlayerState>(ActorInfo->OwnerActor.Get());
	if (!PS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// GA 이벤트 호출 시, 받은 TriggerEventData를 통해 필요한 데이터 분해 
	const auto* TargetData = TriggerEventData->TargetData.Get(0);
	SlotIndex = TargetData->GetHitResult()->Location.X;
	BuyCount = TargetData->GetHitResult()->Location.Y;
	UnitTag = PS->GetShopSlots()[SlotIndex].UnitTag;
	UnitCost = static_cast<float>(PS->GetShopSlots()[SlotIndex].UnitCost);

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const auto CostAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCPlayerAttributeSet>();
		if (!CostAttributeSet || CostAttributeSet->GetPlayerGold() < UnitCost * BuyCount)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (auto GS = GetWorld()->GetGameState<APCCombatGameState>())
		{
			if (BuyCount == 0)
			{
				GS->GetShopManager()->BuyUnit(PS, SlotIndex, UnitTag);
			}
			else
			{
				// 여러개 구매 -> 벤치가 꽉찬 상태이므로, 새로운 유닛 스폰 대신 즉시 레벨업 실행
				GS->GetShopManager()->UnitLevelUp(PS, UnitTag, BuyCount);
			}
		}

		ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTags::GameplayCue_Player_BuyUnit);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_BuyUnit::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	SlotIndex = 0;
	BuyCount = 0;
	UnitCost = 0.f;
	UnitTag = FGameplayTag();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
