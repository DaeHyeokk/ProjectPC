// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/Notify/PCAnimNotifyState_ProjectileBarrage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Animation/Unit/Notify/PCAnimNotify_SpawnProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void UPCAnimNotifyState_ProjectileBarrage::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;
	if (UWorld* World = MeshComp->GetWorld())
	{
		if (FireMode == EProjectileFireMode::AlternateLR)
		{
			UseLeftMap.FindOrAdd(MeshComp) = true;
		}

		SpawnProjectileAndFX(MeshComp);

		FTimerHandle& Handle = TimerMap.FindOrAdd(MeshComp);
		World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateUObject(this, &UPCAnimNotifyState_ProjectileBarrage::SpawnProjectileAndFX, MeshComp),
			FMath::Clamp(FireInterval, 0.02f, 10.f),
			true);
	}
}

void UPCAnimNotifyState_ProjectileBarrage::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;
	if (UWorld* World = MeshComp->GetWorld())
	{
		if (FTimerHandle* Handle = TimerMap.Find(MeshComp))
		{
			World->GetTimerManager().ClearTimer(*Handle);
		}
	}
	TimerMap.Remove(MeshComp);
	UseLeftMap.Remove(MeshComp);
}

void UPCAnimNotifyState_ProjectileBarrage::SpawnProjectileAndFX(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
		return;

	UWorld* World = Owner->GetWorld();
	if (!World)
		return;

	FName SocketName;
	if (FireMode == EProjectileFireMode::Single)
	{
		SocketName = SingleMuzzleSocket;
	}
	else
	{
		const bool bUseLeft = UseLeftMap.FindRef(MeshComp);
		SocketName = bUseLeft ? LeftMuzzleSocket : RightMuzzleSocket;
		UseLeftMap.FindOrAdd(MeshComp) = !bUseLeft;
	}
	
	if (SocketName.IsNone() || !MeshComp->DoesSocketExist(SocketName))
		return;

	// Dedicated Server에서는 이펙트 생성 X
	if (MuzzleFX && Owner->GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleFX,
			MeshComp,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator
			);
	}
	
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AAIController* AIC = OwnerPawn ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
	const UBlackboardComponent* BB = AIC ? AIC->GetBlackboardComponent() : nullptr;
	if (!BB)
		return;

	const AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetUnitKeyName));
	if (!Target)
		return;

	// 발사체는 서버에서만 생성 
	if (Owner->HasAuthority())
	{
		const FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName, RTS_World);
		APCBaseProjectile* Projectile = nullptr;
		
		if (auto* ProjectilePool = World->GetSubsystem<UPCProjectilePoolSubsystem>())
		{
			Projectile = ProjectilePool->SpawnProjectile(
				SocketTransform, UnitTypeTag, ProjectileTypeTag, Owner, Target);
		}

		if (!Projectile)
			return;

		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
		{
			FGameplayEventData Payload;
			Payload.EventTag = EventTag;
			Payload.Instigator = Owner;

			FGameplayAbilityTargetDataHandle Handle;
			FTargetData_Projectile* Data = new FTargetData_Projectile();
			Data->Projectile = Projectile;
			Handle.Add(Data);
			Payload.TargetData = Handle;

			ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}
}
