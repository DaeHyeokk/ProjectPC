// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemInfoWidget.h"

#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_AttributeChange.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"

#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Item/PCItemStatWidget.h"


void UPCItemInfoWidget::Setup(FGameplayTag ItemTag)
{
	// 새롭게 입력된 아이템 태그가 기존과 다를 때만 새롭게 세팅
	if (CachedItemTag == ItemTag) return;

	if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto Item = ItemManager->GetItemData(ItemTag))
		{
			if (Item->IsValid())
			{
				CachedItemTag = ItemTag;
			}
		}
	}

	SetItemInfo();
}

void UPCItemInfoWidget::SetItemInfo()
{
	if (!Img_ItemThumbnail || !Text_ItemName || !Text_ItemUniqueEffect || !ItemStatBox)
		return;

	ItemStatBox->ClearChildren();
	
	if (const auto ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto Item = ItemManager->GetItemData(CachedItemTag))
		{
			if (Item->IsValid())
			{
				Text_ItemName->SetText(FText::FromName(Item->ItemName));
				Text_ItemUniqueEffect->SetText(Item->ItemUniqueEffect);

				// 아이템 이미지 비동기 로드
				FSoftObjectPath TexturePath = Item->ItemTexture.ToSoftObjectPath();
				FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
				TWeakObjectPtr<UPCItemInfoWidget> WeakThis = this;
	
				Streamable.RequestAsyncLoad(TexturePath, [WeakThis, TexturePath]()
				{
					if (WeakThis.IsValid())
					{
						if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
						{
							WeakThis->Img_ItemThumbnail->SetBrushFromTexture(Texture);
						}
					}
				});

				// 아이템 효과 개수만큼 ItemStatWidget을 생성해서 세팅
				if (const auto ItemEffectSpecList = ItemManager->GetItemEffectSpecList(CachedItemTag))
				{
					for (const auto EffectSpec : ItemEffectSpecList->EffectSpecs)
					{
						auto ItemStatWidget = CreateWidget<UPCItemStatWidget>(GetWorld(), ItemStatWidgetClass);
						if (!ItemStatWidget) continue;

						if (const auto Effect = Cast<UPCEffectSpec_AttributeChange>(EffectSpec))
						{
							FString TagString = Effect->EffectCallerTag.ToString();
							ItemStatWidget->Setup(Effect->EffectCallerTag, Effect->EffectMagnitude.ConstantMagnitude, TagString.EndsWith(TEXT("Multiplier")));
							ItemStatBox->AddChild(ItemStatWidget);
						}
					}
				}
			}
		}
	}
}
