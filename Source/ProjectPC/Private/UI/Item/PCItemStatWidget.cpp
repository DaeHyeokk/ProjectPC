// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemStatWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"


void UPCItemStatWidget::Setup(FGameplayTag StatTag, float StatValue, bool bIsMultiplier)
{
	if (!Img_Stat || !Text_StatValue || !DA_StatIcons) return;

	// 스탯 아이콘 비동기 로드
	auto StatIconSoftPtr = DA_StatIcons->GetStatIcon(StatTag);
	if (!StatIconSoftPtr.IsNull())
	{
		FSoftObjectPath TexturePath = StatIconSoftPtr.ToSoftObjectPath();
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		TWeakObjectPtr<UPCItemStatWidget> WeakThis = this;
	
		Streamable.RequestAsyncLoad(TexturePath, [WeakThis, TexturePath]()
		{
			if (WeakThis.IsValid())
			{
				if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
				{
					WeakThis->Img_Stat->SetBrushFromTexture(Texture);
				}
			}
		});
	}

	FString StatValueText;
	if (bIsMultiplier)
	{
		// 퍼센트 증가
		StatValueText = FString::Printf(TEXT("+%d%%"), static_cast<int32>(StatValue));
	}
	else
	{
		// 단순 수치 증가
		StatValueText = FString::Printf(TEXT("+%d"), static_cast<int32>(StatValue));
	}

	Text_StatValue->SetText(FText::FromString(StatValueText));
}
