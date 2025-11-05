// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCPreviewHeroActor.h"

#include "HairStrandsInterface.h"
#include "Animation/Unit/PCPreviewHeroAnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Components/WidgetComponent.h"
#include "UI/Unit/PCHeroStatusBarWidget.h"


// Sets default values
APCPreviewHeroActor::APCPreviewHeroActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(false);
	SetActorEnableCollision(false);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.0f), FRotator(0.f,-90.f,0.f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->bCastDynamicShadow = false;
	MeshComponent->bReceivesDecals = false;

	StatusBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusBar"));
	StatusBarComp->SetupAttachment(MeshComponent);
	StatusBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	StatusBarComp->SetDrawAtDesiredSize(true);
	StatusBarComp->SetVisibility(true);
	StatusBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StatusBarComp->SetPivot({0.5f, 1.f});
}

void APCPreviewHeroActor::SetupSourceSnapshot(FPoseSnapshot& OutSnapshot)
{
	if (USkeletalMeshComponent* SourceMesh = SourceHero.IsValid() ? SourceHero->GetMesh() : nullptr)
	{
		SourceMesh->SnapshotPose(OutSnapshot);
	}
}

void APCPreviewHeroActor::InitializeFromSourceHero(APCHeroUnitCharacter* InSourceHero, const TSubclassOf<UAnimInstance>& SourcePreviewAnimBP, const TSubclassOf<UUserWidget>& SourceStatusBarClass)
{
	SourceHero = InSourceHero;

	if (!InSourceHero)
		return;

	if (USkeletalMeshComponent* SrcMesh = InSourceHero->GetMesh())
	{
		MeshComponent->SetRelativeScale3D(SrcMesh->GetRelativeScale3D());
		MeshComponent->SetSkeletalMesh(SrcMesh->GetSkeletalMeshAsset());
		MeshComponent->SetAnimInstanceClass(SourcePreviewAnimBP);

		MeshComponent->HideBoneByName(TEXT("sheild_main"), PBO_None); // Steel 방패 제거 
		MeshComponent->HideBoneByName(TEXT("arm_chain_long_r_01"), PBO_None); // Riktor 오른쪽 체인 비정상적인 메쉬 제거
		MeshComponent->HideBoneByName(TEXT("wing_l_01"), PBO_None); // Serath 날개 제거
		MeshComponent->HideBoneByName(TEXT("wing_r_01"), PBO_None);
		MeshComponent->HideBoneByName(TEXT("ghostbeast_root"), PBO_None); // Khaimera Ghost Beast 소켓 제거
	}

	if (!StatusBarComp || !InSourceHero || !SourceStatusBarClass)
		return;

	StatusBarComp->SetWidgetClass(SourceStatusBarClass);
	if (!StatusBarComp->GetWidget())
	{
		StatusBarComp->InitWidget();
	}

	FName StatusBarSocketName = SourceHero->GetStatusBarSocketName();
	if (MeshComponent->DoesSocketExist(StatusBarSocketName))
	{
		StatusBarComp->AttachToComponent(
			MeshComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			StatusBarSocketName);

		StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,60.f));

		const UPCHeroStatusBarWidget* PreviewStatusBar = Cast<UPCHeroStatusBarWidget>(StatusBarComp->GetWidget());
		const UPCHeroStatusBarWidget* SourceStatusBar = Cast<UPCHeroStatusBarWidget>(SourceHero->GetStatusBarComponent()->GetWidget());

		if (PreviewStatusBar && SourceStatusBar)
		{
			PreviewStatusBar->CopyVariantBySourceStatusBar(SourceStatusBar);
		}
	}
}

void APCPreviewHeroActor::UpdateLocation(const FVector& WorldLoc)
{
	SetActorLocation(WorldLoc);
}

void APCPreviewHeroActor::TearDown()
{
	//SetActorHiddenInGame(true);
	Destroy();
}
