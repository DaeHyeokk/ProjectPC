// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"


// void UPCPlayerRowWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
// {
// 	if (!NewPlayerState) return;
// 	CachedPlayerState = NewPlayerState;
//
// 	if (auto ASC = NewPlayerState->GetAbilitySystemComponent())
// 	{
// 		if (auto AttributeSet = NewPlayerState->GetAttributeSet())
// 		{
// 			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerHPAttribute())
// 			.AddUObject(this, &UPCPlayerRowWidget::OnPlayerHPChanged);
// 		}
// 	}
//
// 	SetupPlayerInfo();
// }

void UPCPlayerRowWidget::SetupPlayerInfo(FString NewPlayerName, float NewPlayerHP, FGameplayTag NewPlayerCharacterTag)
{
	if (!PlayerName || !PlayerHP || !CircularHPBar || !Img_Portrait) return; 

	// 플레이어 이름 세팅
	auto NameText = FString::Printf(TEXT("%s"), *NewPlayerName);
	PlayerName->SetText(FText::FromString(NameText));

	// 플레이어 체력바 세팅
	auto HP = NewPlayerHP;
	auto HPPercent = HP / 100.f;
	SetHP(HPPercent);

	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
	PlayerHP->SetText(FText::FromString(HPText));

	// 플레이어 초상화 세팅
	auto PortraitSoftPtr = PlayerPortrait->GetPlayerPortrait(NewPlayerCharacterTag);
	FSoftObjectPath TexturePath = PortraitSoftPtr.ToSoftObjectPath();
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	
	Streamable.RequestAsyncLoad(TexturePath, [this, TexturePath]()
	{
		if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
		{
			Img_Portrait->SetBrushFromTexture(Texture);
		}
	});
}

void UPCPlayerRowWidget::UpdatePlayerHP(float NewPlayerHP)
{
	if (!PlayerHP || !CircularHPBar) return;
	
	auto HP = NewPlayerHP;
	auto HPPercent = HP / 100.f;
	SetHP(HPPercent);

	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
	PlayerHP->SetText(FText::FromString(HPText));

	if (HP <= 0)
	{
		if (Img_Portrait)
		{
			Img_Portrait->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
		}
	}
}

void UPCPlayerRowWidget::SetHP_Implementation(float HPPercent)
{
}

// void UPCPlayerRowWidget::OnPlayerHPChanged(const FOnAttributeChangeData& Data)
// {
// 	if (!PlayerHP || !CircularHPBar) return;
// 	
// 	auto HP = Data.NewValue;
// 	auto HPPercent = HP / 100.f;
// 	SetHP(HPPercent);
//
// 	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
// 	PlayerHP->SetText(FText::FromString(HPText));
//
// 	if (HP <= 0)
// 	{
// 		if (Img_Portrait)
// 		{
// 			Img_Portrait->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
// 		}
// 	}
// }
