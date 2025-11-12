// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerOverheadWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "GameFramework/PlayerState/PCPlayerState.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Components/WidgetComponent.h"

void UPCPlayerOverheadWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// if (auto OwnerPawn = Cast<APCPlayerCharacter>(GetOwningPlayerPawn()))
	// {
	// 	if (auto PS = OwnerPawn->GetPlayerState<APCPlayerState>())
	// 	{
	// 		BindToPlayerState(CachedPlayerState);
	// 	}
	// }
	
	// if (CachedPlayerState)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("OverheadWidget Construct : %s"), *CachedPlayerState->LocalUserId)
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("(OverheadWidget Construct) CachedPlayerState is nullptr"));
	// }

	if (auto WidgetComp = GetTypedOuter<UWidgetComponent>())
	{
		if (auto OwnerChar = Cast<APCPlayerCharacter>(WidgetComp->GetOwner()))
		{
			BindToPlayerState(OwnerChar->GetPlayerState<APCPlayerState>());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("(OverheadWidget Construct) Owner is nullptr"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(OverheadWidget Construct) WidgetComp is nullptr"));
	}
}

void UPCPlayerOverheadWidget::NativeDestruct()
{
	
}

void UPCPlayerOverheadWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("(OverheadWidget Construct) NewPlayerState is nullptr"));
		return;
	}

	CachedPlayerState = NewPlayerState;
	
	// 플레이어 어트리뷰트 (HP, Level) 구독
	if (!OnPlayerLevelChangeHandle.IsValid() || !OnPlayerHPChangeHandle.IsValid())
	{
		if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
		{
			if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
			{
				OnPlayerLevelChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerLevelAttribute())
				.AddUObject(this, &UPCPlayerOverheadWidget::OnPlayerLevelChanged);
				OnPlayerHPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerHPAttribute())
				.AddUObject(this, &UPCPlayerOverheadWidget::OnPlayerHPChanged);
			}
		}
	}

	SetupPlayerInfo();
}

void UPCPlayerOverheadWidget::SetupPlayerInfo()
{
	if (!PlayerName || !PlayerLevel || !HPBar) return; 
	if (!CachedPlayerState) return;
	
	// 플레이어 이름 세팅
	auto NameText = FString::Printf(TEXT("%s"), *CachedPlayerState->LocalUserId);
	PlayerName->SetText(FText::FromString(NameText));
	
	const auto AttributeSet = CachedPlayerState->GetAttributeSet();
	if (!AttributeSet) return;

	// 플레이어 레벨 세팅
	auto Level = static_cast<int32>(AttributeSet->GetPlayerLevel());
	auto LevelText = FString::Printf(TEXT("%d"), Level);
	PlayerLevel->SetText(FText::FromString(LevelText));
	
	// 플레이어 체력바 세팅
	auto PlayerIndex = CachedPlayerState->SeatIndex;
	if (PlayerColors.IsValidIndex(PlayerIndex))
	{
		HPBar->SetFillColorAndOpacity(PlayerColors[PlayerIndex]);
	}
	
	auto HP = AttributeSet->GetPlayerHP();
	auto HPPercent = HP / 100.f;

	if (HPPercent < 1.f)
	{
		HPBar->SetPercent(HPPercent);
	}
	else
	{
		HPBar->SetPercent(1.f);
	}
}

void UPCPlayerOverheadWidget::OnPlayerLevelChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerLevel) return;

	auto Level = static_cast<int32>(Data.NewValue);
	auto LevelText = FString::Printf(TEXT("%d"), Level);
	PlayerLevel->SetText(FText::FromString(LevelText));
}

void UPCPlayerOverheadWidget::OnPlayerHPChanged(const FOnAttributeChangeData& Data)
{
	if (!HPBar) return;

	auto HP = Data.NewValue;
	auto HPPercent = HP / 100.f;

	if (HPPercent < 1.f)
	{
		HPBar->SetPercent(HPPercent);
	}
	else
	{
		HPBar->SetPercent(1.f);
	}
}
