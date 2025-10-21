// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCCarouselUnitWidget.h"
#include "Character/Unit/PCCommonUnitCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"
#include "UI/Unit/PCHeroStatusHoverPanel.h"


void UPCCarouselUnitWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!UnitInfoButton) return;
	UnitInfoButton->OnClicked.AddDynamic(this, &UPCCarouselUnitWidget::CreateUnitInfoWidget);
}


void UPCCarouselUnitWidget::SetItemImg(FGameplayTag ItemTag)
{
	if (!ItemImg) return;

	if (const auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto NewItem = ItemManagerSubsystem->GetItemData(ItemTag))
		{
			if (NewItem->IsValid())
			{
				FSoftObjectPath TexturePath = NewItem->ItemTexture.ToSoftObjectPath();

				if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
				{
					ItemImg->SetBrushFromTexture(Texture);
				}
			}
		}
	}
}

void UPCCarouselUnitWidget::SetUnit(APCCommonUnitCharacter* Unit)
{
	if (!Unit) return;

	CachedHero = Unit;
}

void UPCCarouselUnitWidget::CreateUnitInfoWidget()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController) return;

	if (APCCombatPlayerController* PCPlayerController = Cast<APCCombatPlayerController>(PlayerController))
	{
		if (UPCHeroStatusHoverPanel* HeroStatusWidget =  PCPlayerController->GetPlayerMainWidget()->GetHeroStatusWidget())
		{
			HeroStatusWidget->ShowPanelForHero(CachedHero.Get());
		}
	}
}
