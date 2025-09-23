// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/Notify/PCAnimNotify_SpawnProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BaseGameplayTags.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"


void UPCAnimNotify_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
		return;

	if (SocketName.IsNone() || !MeshComp->DoesSocketExist(SocketName))
		return;

	const FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName, RTS_World);
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AAIController* AIC = OwnerPawn ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
	const UBlackboardComponent* BB = AIC ? AIC->GetBlackboardComponent() : nullptr;
	if (!BB)
		return;

	const AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetUnitKeyName));
	if (!Target)
		return;

	APCBaseProjectile* Projectile = nullptr;
	if (UWorld* World = Owner->GetWorld())
	{
		if (auto* ProjectilePool = World->GetSubsystem<UPCProjectilePoolSubsystem>())
		{
			Projectile = ProjectilePool->SpawnProjectile(
				SocketTransform, UnitTypeTag, ProjectileTypeTag, Owner, Target);
		}
	}

	if (!Projectile)
		return;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		FGameplayEventData Payload;
		Payload.EventTag = UnitGameplayTags::Unit_Event_SpawnProjectileSucceed;
		Payload.Instigator = Owner;
		Payload.OptionalObject = Projectile;

		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}
