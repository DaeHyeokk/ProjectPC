// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GC/PCGameplayCue_DamageText.h"

#include "AbilitySystem/Unit/ExecutionCalculation/UPCUnitDamageExec.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DynamicUIActor/PCDamageTextPayload.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitDamageTextSpawnSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UPCGameplayCue_DamageText::OnExecute_Implementation(AActor* MyTarget,
                                                         const FGameplayCueParameters& Parameters) const
{
	// 클라 본인이 소환한 유닛이 입히는 데미지만 UI 표시
	const APCBaseUnitCharacter* Instigator = Cast<APCBaseUnitCharacter>(Parameters.Instigator);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	const APCPlayerState* LocalPS = PC ? Cast<APCPlayerState>(PC->PlayerState.Get()) : nullptr;
	if (!Instigator || !LocalPS || Instigator->GetTeamIndex() != LocalPS->SeatIndex)
		return false;
		
	if (!MyTarget)
		return false;
	
	const USceneComponent* AttachComp = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
	if (!AttachComp)
		AttachComp = MyTarget->GetRootComponent();
	if (!AttachComp)
		return false;

	const float Damage = Parameters.RawMagnitude;
	const bool bCrit = Parameters.NormalizedMagnitude >= 0.5f;
	const EDamageType DamageType = static_cast<EDamageType>(Parameters.GameplayEffectLevel);
	
	if (UWorld* World = MyTarget->GetWorld())
	{
		if (auto* Subsystem = World->GetSubsystem<UPCUnitDamageTextSpawnSubsystem>())
		{
			FDamageTextInitParams InitParams;
			InitParams.Damage = Damage;
			InitParams.bCritical = bCrit;
			InitParams.DamageTypeTag = PhysicalDamageTypeTag;
			switch (DamageType)
			{
			case Physical:
				InitParams.DamageTypeTag = PhysicalDamageTypeTag;
				break;
			case Magic:
				InitParams.DamageTypeTag = MagicDamageTypeTag;
				break;
			default:
				break;
			}
			
			Subsystem->SpawnDamageText(AttachComp, InitParams);
			return true;
		}
	}
	
	return false;
}
