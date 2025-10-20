// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemRecipeRowWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"

#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"


void UPCItemRecipeRowWidget::Setup(FGameplayTag BaseItemTag, FGameplayTag AdvancedItemTag)
{
	if (!Img_BaseItemThumbnail || !Img_AdvancedItemThumbnail || !Text_AdvancedItemName)
		return;
	
	if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		const auto BaseItem = ItemManager->GetItemData(BaseItemTag);
		const auto AdvancedItem = ItemManager->GetItemData(AdvancedItemTag);

		if (BaseItem && BaseItem->IsValid() && AdvancedItem && AdvancedItem->IsValid())
		{
			TArray<FSoftObjectPath> ItemTexturePath;
			ItemTexturePath.Add(BaseItem->ItemTexture.ToSoftObjectPath());
			ItemTexturePath.Add(AdvancedItem->ItemTexture.ToSoftObjectPath());
			
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			TWeakObjectPtr<UPCItemRecipeRowWidget> WeakThis = this;
	
			Streamable.RequestAsyncLoad(ItemTexturePath, [WeakThis, ItemTexturePath]()
			{
				if (WeakThis.IsValid())
				{
					if (UTexture2D* Texture = Cast<UTexture2D>(ItemTexturePath[0].ResolveObject()))
					{
						WeakThis->Img_BaseItemThumbnail->SetBrushFromTexture(Texture);
					}
					
					if (UTexture2D* Texture = Cast<UTexture2D>(ItemTexturePath[1].ResolveObject()))
					{
						WeakThis->Img_AdvancedItemThumbnail->SetBrushFromTexture(Texture);
					}
				}
			});

			Text_AdvancedItemName->SetText(FText::FromName(AdvancedItem->ItemName));
		}
	}
}