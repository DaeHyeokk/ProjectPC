// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CenterPlayerWidget.generated.h"

class UTextBlock;
class UImage;
class UBorder;
class APCPlayerState;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UCenterPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 바인딩 (UMG에서 위의 이름으로 만들어 둠)
	UPROPERTY(meta=(BindWidget))
	UBorder* RootFrame = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UImage*  ImgAvatar = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UBorder* LevelBadge = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TbLevel = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UImage*  ImgCrown = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TbName = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TbSubtitle = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UBorder* Slot_LittleLegend = nullptr;

	UPROPERTY(meta=(BindWidget))
	UImage* Img_Ready = nullptr;

	// 스타일(에셋이 없으면 색상으로 대체)
	UPROPERTY(EditAnywhere, Category="Style") UTexture2D* FrameTexture = nullptr;
	UPROPERTY(EditAnywhere, Category="Style") FMargin     FrameMargin = FMargin(8.f);
	UPROPERTY(EditAnywhere, Category="Style") FLinearColor FrameTint = FLinearColor(1,1,1,0.9f);

	UPROPERTY(EditAnywhere, Category="Style") UTexture2D* CrownTexture = nullptr;
	UPROPERTY(EditAnywhere, Category="Style") UTexture2D* DefaultAvatar = nullptr;

	// 데이터 세팅 API
	UFUNCTION(BlueprintCallable) void SetPlayer(APCPlayerState* PS);
	UFUNCTION(BlueprintCallable) void SetDisplayName(const FText& Name);
	UFUNCTION(BlueprintCallable) void SetLevel(int32 Level);
	UFUNCTION(BlueprintCallable) void SetLeader(bool bLeader);
	UFUNCTION(BlueprintCallable) void SetAvatar(UTexture2D* AvatarTex);
	UFUNCTION(BlueprintCallable) void SetLittleLegendEmpty(bool bEmpty);

protected:
	virtual void NativeConstruct() override;

private:
	TWeakObjectPtr<APCPlayerState> PlayerPS;
	int32  CachedLevel = 1;
	bool   bIsLeader = false;
	UTexture2D* CachedAvatar = nullptr;

	void ApplyStyleOnce();
	static FSlateBrush MakeBrush(UTexture2D* Tex, FVector2D Size=FVector2D::ZeroVector,
								 FMargin Margin=FMargin(0), FLinearColor Tint=FLinearColor::White);
};
	


