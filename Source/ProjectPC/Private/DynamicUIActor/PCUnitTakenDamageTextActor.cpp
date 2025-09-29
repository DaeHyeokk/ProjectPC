// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicUIActor/PCUnitTakenDamageTextActor.h"
#include "ProjectPC/Public/UI/Unit/PCUnitTakenDamageText.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/WorldSubsystem/PCUnitDamageTextSpawnSubsystem.h"

APCUnitTakenDamageTextActor::APCUnitTakenDamageTextActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetActorEnableCollision(false);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawSize(FVector2D(220.f, 100.f));
	WidgetComponent->SetTwoSided(false);
	WidgetComponent->SetGenerateOverlapEvents(false);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetCastShadow(false);
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
}

void APCUnitTakenDamageTextActor::InitDamageTextActor(const USceneComponent* InAttachComp,
	const FDamageTextInitParams& InParams)
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
			UMG = CreateWidget<UPCUnitTakenDamageText>(GetWorld(), WidgetComponent->GetWidgetClass());
			WidgetComponent->SetWidget(UMG);
		}
		
		CachedWidget = Cast<UPCUnitTakenDamageText>(WidgetComponent->GetWidget());
		if (CachedWidget)
		{
			// 중복 바인딩 방지
			CachedWidget->OnFinished.RemoveAll(this);
			CachedWidget->OnFinished.AddDynamic(this, &APCUnitTakenDamageTextActor::OnWidgetFinished);
		}
	}
	else
	{
		// 재사용 시 바인딩 클린업
		CachedWidget->OnFinished.RemoveAll(this);
		CachedWidget->OnFinished.AddDynamic(this, &APCUnitTakenDamageTextActor::OnWidgetFinished);
	}

	// 초기화 호출
	if (CachedWidget)
	{
		CachedWidget->InitializeDamageText(InParams.Damage, InParams.bCritical, InParams.DamageTypeTag);
	}
}

void APCUnitTakenDamageTextActor::ReturnToPool()
{
	LiftOwner = nullptr;

	SetActorHiddenInGame(true);

	UWorld* World = GetWorld();
	UPCUnitDamageTextSpawnSubsystem* DamageTextSpawnSubsystem = World ? World->GetSubsystem<UPCUnitDamageTextSpawnSubsystem>() : nullptr;
	if (DamageTextSpawnSubsystem)
	{
		DamageTextSpawnSubsystem->ReturnToPool(this);
	}
	else // 만약 풀이 없으면 파괴 (방어 코드)
	{
		Destroy();
	}
}

void APCUnitTakenDamageTextActor::OnWidgetFinished()
{
	ReturnToPool();
}