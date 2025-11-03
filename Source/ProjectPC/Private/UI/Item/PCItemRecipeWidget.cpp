// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemRecipeWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Item/PCItemRecipeRowWidget.h"


void UPCItemRecipeWidget::Setup(FGameplayTag BaseItemTag)
{
	// 새롭게 입력된 아이템 태그가 기존과 다를 때만 새롭게 세팅
	if (CachedBaseItemTag == BaseItemTag) return;

	if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto BaseItem = ItemManager->GetItemData(BaseItemTag))
		{
			if (BaseItem->IsValid())
			{
				CachedBaseItemTag = BaseItemTag;
			}
		}
	}

	SetRecipeRow();
}

void UPCItemRecipeWidget::SetRecipeRow()
{
	if (!ItemInfoBox) return;

	ItemInfoBox->ClearChildren();
	
	if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto BaseItem = ItemManager->GetItemData(CachedBaseItemTag))
		{
			if (BaseItem->IsValid())
			{
				auto ItemRecipeMap = ItemManager->GetItemRecipe(CachedBaseItemTag);

				for (auto ItemRecipe : ItemRecipeMap)
				{
					auto ItemRecipeRowWidget = CreateWidget<UPCItemRecipeRowWidget>(GetWorld(), ItemRecipeRowWidgetClass);
					if (!ItemRecipeRowWidget) continue;

					ItemRecipeRowWidget->Setup(ItemRecipe.Key, ItemRecipe.Value);
					ItemInfoBox->AddChild(ItemRecipeRowWidget);
				}
			}
		}
	}
}
