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
		MeshComponent->SetSkeletalMesh(SrcMesh->GetSkeletalMeshAsset());
		MeshComponent->SetAnimInstanceClass(SourcePreviewAnimBP);
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
		StatusBarComp->SetRelativeLocation(FVector(0.f,0.f,30.f));

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
	Destroy();
}
