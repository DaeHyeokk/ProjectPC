// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void UPCPlayerRowWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
	CachedPlayerState = NewPlayerState;

	if (auto ASC = NewPlayerState->GetAbilitySystemComponent())
	{
		if (auto AttributeSet = NewPlayerState->GetAttributeSet())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerHPAttribute())
			.AddUObject(this, &UPCPlayerRowWidget::OnPlayerHPChanged);
		}
	}

	SetupPlayerInfo();
}

void UPCPlayerRowWidget::SetupPlayerInfo()
{
	if (!PlayerName || !PlayerHP || !CircularHPBar || !Img_Portrait) return; 
	if (!CachedPlayerState) return;

	// 플레이어 이름 세팅
	auto NameText = FString::Printf(TEXT("%s"), *CachedPlayerState->LocalUserId);
	PlayerName->SetText(FText::FromString(NameText));

	// 플레이어 체력바 세팅
	const auto AttributeSet = CachedPlayerState->GetAttributeSet();
	if (!AttributeSet) return;

	auto HP = AttributeSet->GetPlayerHP();
	auto HPPercent = HP / 100.f;
	SetHP(HPPercent);

	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
	PlayerHP->SetText(FText::FromString(HPText));

	// 플레이어 초상화 세팅
	FGameplayTagContainer PlayerTags;
	FGameplayTag CharacterTag;
	
	auto ASC = CachedPlayerState->GetAbilitySystemComponent();
	if (!ASC) return;

	ASC->GetOwnedGameplayTags(PlayerTags);

	for (const FGameplayTag& Tag : PlayerTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Player.Type"))))
		{
			CharacterTag = Tag;
			break;
		}
	}
	
	auto PortraitSoftPtr = PlayerPortrait->GetPlayerPortrait(CharacterTag);
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

void UPCPlayerRowWidget::SetHP_Implementation(float HPPercent)
{
}

void UPCPlayerRowWidget::OnPlayerHPChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerHP || !CircularHPBar) return;
	
	auto HP = Data.NewValue;
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
