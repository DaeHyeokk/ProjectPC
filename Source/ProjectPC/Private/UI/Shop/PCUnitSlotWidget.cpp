// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCUnitSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


bool UPCUnitSlotWidget::Initialize()
{
	bool SuperSuccess = Super::Initialize();
	if (!SuperSuccess) return false;

	if (!Btn_UnitSlot) return false;
	Btn_UnitSlot->OnClicked.AddDynamic(this, &UPCUnitSlotWidget::OnClickedUnitSlot);

	return true;
}

void UPCUnitSlotWidget::Setup(const FPCShopUnitData& UnitData, bool bIsShopSlot, int32 NewSlotIndex)
{
	if (!Text_UnitName || !Text_Cost || !Text_Species || !Text_Job
		|| !Img_UnitThumbnail || !Img_CostBorder || !Img_Species || !Img_Job
		|| !SynergyInfo) return;
	
	SlotIndex = NewSlotIndex;
	UnitCost = UnitData.UnitCost;

	Text_UnitName->SetText(FText::FromName(UnitData.UnitName));
	Text_Cost->SetText(FText::AsNumber(UnitData.UnitCost));

	FPCSynergyUIRow SpeciesRow;
	FPCSynergyUIRow JobRow;
	if (SynergyInfo->Resolve(UnitData.UnitSpeciesTag, SpeciesRow)
		&& SynergyInfo->Resolve(UnitData.UnitJobTag, JobRow))
	{
		Text_Species->SetText(SpeciesRow.DisplayName);
		Text_Job->SetText(JobRow.DisplayName);

		Img_Species->SetBrush(SpeciesRow.Icon);
		Img_Species->SetBrushTintColor(FSlateColor(FLinearColor::White));
		Img_Job->SetBrush(JobRow.Icon);
		Img_Job->SetBrushTintColor(FSlateColor(FLinearColor::White));
	}
	
	// 유닛 썸네일 세팅 (SoftObjectPtr 비동기 로드)
	FSoftObjectPath TexturePath = UnitData.UnitTexture.ToSoftObjectPath();
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TWeakObjectPtr<UPCUnitSlotWidget> WeakThis = this;
	
	Streamable.RequestAsyncLoad(TexturePath, [WeakThis, TexturePath]()
	{
		if (WeakThis.IsValid())
		{
			if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
			{
				WeakThis->Img_UnitThumbnail->SetBrushFromTexture(Texture);
			}
		}
	});
	
	UTexture2D* BorderTexture = nullptr;
	switch (UnitData.UnitCost)
	{
	case 1:
		BorderTexture = Cost1Border;
		break;
	case 2:
		BorderTexture = Cost2Border;
		break;
	case 3:
		BorderTexture = Cost3Border;
		break;
	case 4:
		BorderTexture = Cost4Border;
		break;
	case 5:
		BorderTexture = Cost5Border;
		break;
	default:
		break;
	}

	if (BorderTexture)
	{
		Img_CostBorder->SetBrushFromTexture(BorderTexture);
	}

	if (bIsShopSlot)
	{
		SetupButton();
	}
	else
	{
		if (Btn_UnitSlot)
		{
			Btn_UnitSlot->SetIsEnabled(false);
		}
	}
}

void UPCUnitSlotWidget::SetupButton()
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	if (auto PC = GetOwningPlayer())
	{
		if (auto PS = PC->GetPlayerState<APCPlayerState>())
		{
			if (auto AttributeSet = PS->GetAttributeSet())
			{
				if (static_cast<int32>(AttributeSet->GetPlayerGold()) < UnitCost)
				{
					Btn_UnitSlot->SetIsEnabled(false);
					SetRenderOpacity(0.3f);
				}
				else
				{
					Btn_UnitSlot->SetIsEnabled(true);
					SetRenderOpacity(1.f);
				}
			}
		}
	}
}

void UPCUnitSlotWidget::OnClickedUnitSlot()
{
	if (auto PC = Cast<APCCombatPlayerController>(GetOwningPlayer()))
	{
		PC->ShopRequest_BuyUnit(SlotIndex);
	}
}

void UPCUnitSlotWidget::SetSlotHidden(bool IsHidden)
{
	if (IsHidden)
	{
		this->SetVisibility(ESlateVisibility::Hidden);
	}
}
