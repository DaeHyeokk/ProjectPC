// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/PCPreviewHeroAnimInstance.h"


void UPCPreviewHeroAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	CachedPreviewHero = Cast<APCPreviewHeroActor>(GetOwningActor());
	if (CachedPreviewHero.IsValid())
	{
		CachedPreviewHero->SetupSourceSnapshot(SourceSnapshot);
	}
}

void UPCPreviewHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedPreviewHero.IsValid())
	{
		CachedPreviewHero = Cast<APCPreviewHeroActor>(GetOwningActor());
	}

	if (CachedPreviewHero.IsValid())
	{
		CachedPreviewHero->SetupSourceSnapshot(SourceSnapshot);
	}
}
