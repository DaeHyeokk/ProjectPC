// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicUIActor/PCUnitCombatTextActor.h"

#include "BaseGameplayTags.h"
#include "ProjectPC/Public/UI/Unit/PCUnitCombatText.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/WorldSubsystem/PCUnitCombatTextSpawnSubsystem.h"

APCUnitCombatTextActor::APCUnitCombatTextActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetActorEnableCollision(false);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	//WidgetComponent->SetDrawSize(FVector2D(220.f, 100.f));
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetTwoSided(false);
	WidgetComponent->SetGenerateOverlapEvents(false);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetCastShadow(false);
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
}

void APCUnitCombatTextActor::InitCombatTextActor(const USceneComponent* InAttachComp,
	const FCombatTextInitParams& InParams)
{
	// 위치 계산
	FVector SpawnLoc = InAttachComp->GetSocketLocation(InParams.AttachSocketName);
	SpawnLoc.Z += InParams.VerticalOffset;

	// 흔들림 추가
	const float J = InParams.RandomXYJitter;
	SpawnLoc.X += FMath::FRandRange(-J, J);
	SpawnLoc.Y += FMath::FRandRange(-J, J);

	SetActorLocation(SpawnLoc);
	SetActorHiddenInGame(false);

	// 위젯 생성/확보
	if (!CachedWidget)
	{
		UUserWidget* UMG = WidgetComponent->GetWidget();
		if (!UMG && WidgetComponent->GetWidgetClass())
		{
			UMG = CreateWidget<UPCUnitCombatText>(GetWorld(), WidgetComponent->GetWidgetClass());
			WidgetComponent->SetWidget(UMG);
		}
		
		CachedWidget = Cast<UPCUnitCombatText>(WidgetComponent->GetWidget());
		if (CachedWidget)
		{
			// 중복 바인딩 방지
			CachedWidget->OnFinished.RemoveAll(this);
			CachedWidget->OnFinished.AddDynamic(this, &APCUnitCombatTextActor::OnWidgetFinished);
		}
	}
	else
	{
		// 재사용 시 바인딩 클린업
		CachedWidget->OnFinished.RemoveAll(this);
		CachedWidget->OnFinished.AddDynamic(this, &APCUnitCombatTextActor::OnWidgetFinished);
	}

	// 초기화 호출
	if (CachedWidget)
	{
		if (InParams.CombatTextTypeTag.MatchesTag(UnitGameplayTags::Unit_CombatText_Heal))
		{
			CachedWidget->InitializeHealText(InParams.Value);	
		}
		else
		{
			CachedWidget->InitializeDamageText(InParams.Value, InParams.bCritical, InParams.CombatTextTypeTag);
		}
	}
}

void APCUnitCombatTextActor::ReturnToPool()
{
	LiftOwner = nullptr;

	SetActorHiddenInGame(true);

	UWorld* World = GetWorld();
	UPCUnitCombatTextSpawnSubsystem* DamageTextSpawnSubsystem = World ? World->GetSubsystem<UPCUnitCombatTextSpawnSubsystem>() : nullptr;
	if (DamageTextSpawnSubsystem)
	{
		DamageTextSpawnSubsystem->ReturnToPool(this);
	}
	else // 만약 풀이 없으면 파괴 (방어 코드)
	{
		Destroy();
	}
}

void APCUnitCombatTextActor::OnWidgetFinished()
{
	ReturnToPool();
}