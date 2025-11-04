// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/Notify/PCAnimNotify_SpawnParticleAtTargetSocket.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BaseGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystem.h"


void UPCAnimNotify_SpawnParticleAtTargetSocket::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !Particle)
		return;

	AActor* Owner = MeshComp->GetOwner();
	// 서버에서만 실행 (AI Controller는 서버에만 존재), 이펙트는 Gameplay Cue로 클라에 생성
	if (!Owner || !Owner->HasAuthority())
		return;
	
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AAIController* AIC = OwnerPawn ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
	const UBlackboardComponent* BB = AIC ? AIC->GetBlackboardComponent() : nullptr;
	if (!BB)
		return;

	const ACharacter* Target = Cast<ACharacter>(BB->GetValueAsObject(TargetUnitKeyName));
	if (!Target)
		return;

	USkeletalMeshComponent* TargetMeshComp = Target->GetMesh();
	if (!TargetMeshComp)
		return;
	
	if (TargetSocketName.IsNone() || !TargetMeshComp->DoesSocketExist(TargetSocketName))
		return;
	
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
	{
		FGameplayCueParameters Params;
		Params.TargetAttachComponent = TargetMeshComp;
		Params.SourceObject = Particle.Get();

		FGameplayEffectContextHandle Ctx = TargetASC->MakeEffectContext();
		FHitResult HitResult;
		HitResult.BoneName = TargetSocketName;
		Ctx.AddHitResult(HitResult);
		Params.EffectContext = Ctx;
		
		TargetASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_VFX_Unit_PlayEffectAtSocket, Params);
	}
	
}
