// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemRecipeWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Item/PCItemRecipeRowWidget.h"


void UPCItemRecipeWidget::Setup(FGameplayTag BaseItemTag)
{
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
