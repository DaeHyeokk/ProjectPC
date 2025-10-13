// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCUnitSlotWidget.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"

#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


bool UPCUnitSlotWidget::Initialize()
{
	bool SuperSuccess = Super::Initialize();
	if (!SuperSuccess) return false;

	if (!Btn_UnitSlot) return false;
	Btn_UnitSlot->OnClicked.AddDynamic(this, &UPCUnitSlotWidget::OnClickedUnitSlot);

	return true;
}

void UPCUnitSlotWidget::Setup(FPCShopUnitData UnitData, int32 NewSlotIndex)
{
	if (!Text_UnitName || !Text_Cost || !Img_UnitThumbnail || !Img_CostBorder) return;
	
	SlotIndex = NewSlotIndex;
	UnitCost = UnitData.UnitCost;
	
	Text_UnitName->SetText(FText::FromName(UnitData.UnitName));
	Text_Cost->SetText(FText::AsNumber(UnitData.UnitCost));

	FSoftObjectPath TexturePath = UnitData.UnitTexture.ToSoftObjectPath();
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	
	Streamable.RequestAsyncLoad(TexturePath, [this, TexturePath]()
	{
		if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
		{
			Img_UnitThumbnail->SetBrushFromTexture(Texture);
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

	SetupButton();
}

void UPCUnitSlotWidget::SetupButton()
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	if (auto PS = GetOwningPlayer()->GetPlayerState<APCPlayerState>())
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
