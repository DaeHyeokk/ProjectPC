// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_GrantTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BaseGameplayTags.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

FActiveGameplayEffectHandle UPCEffectSpec_GrantTag::ApplyEffectImpl(UAbilitySystemComponent* SourceASC,
	const AActor* Target, int32 EffectLevel)
{
	FActiveGameplayEffectHandle OutHandle;
	
	if (!SourceASC || !Target || !Target->HasAuthority() || !GrantTag.IsValid())
		return OutHandle;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return OutHandle;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
		return OutHandle;

	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;

	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;

	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;
	
	FGameplayEffectSpec& Spec = *SpecHandle.Data.Get();
	ApplyDurationOptions(Spec);

	// DynamicGrantTags에 부여할 GrantTag를 추가하는 방식으로
	// GE를 적용하면 태그가 부여되도록 구현
	SpecHandle.Data->DynamicGrantedTags.AddTag(GrantTag);

	if (TargetGroup == EEffectTargetGroup::Self)
	{
		OutHandle = SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		OutHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);	
	}
	
	return OutHandle;
}

TSubclassOf<UGameplayEffect> UPCEffectSpec_GrantTag::ResolveGEClass(const UWorld* World)
{
	if (CachedGEClass)
		return CachedGEClass;

	if (!World)
		return nullptr;
	
	if (auto* GERegistrySubsystem = World->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		const FGameplayTag EffectClassTag = bUseDurationSetByCaller ? DurationGrantTagClassTag : InfiniteGrantTagClassTag;
		if (TSubclassOf<UGameplayEffect> GEClass = GERegistrySubsystem->GetGEClass(EffectClassTag))
		{
			CachedGEClass = GEClass;
			return GEClass;
		}
	}

	return nullptr;
}
