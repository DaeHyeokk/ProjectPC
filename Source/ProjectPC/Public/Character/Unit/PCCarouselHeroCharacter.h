// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PCCommonUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "DataAsset/Unit/PCDataAsset_UnitDefinition.h"
#include "GameFramework/Character.h"
#include "PCCarouselHeroCharacter.generated.h"

class APCCarouselRing;
class APCPlayerCharacter;
class UPCHeroUnitAttributeSet;
class UWidgetComponent;
class UPCDataAsset_UnitDefinition;
class UPCDataAsset_HeroUnitData;
class UPCDataAsset_UnitAnimSet;

UCLASS()
class PROJECTPC_API APCCarouselHeroCharacter : public APCCommonUnitCharacter
{
	GENERATED_BODY()

public:
	APCCarouselHeroCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComp;}

	void InitAttributeSet();
	
	const UPCDataAsset_HeroUnitData* GetHeroUnitDataAsset() const { return HeroData; }
	void SetHeroUnitDataAsset(UPCDataAsset_HeroUnitData* InHeroData);
	
	FGameplayTag GetUnitTag() const { return UnitTag; }
	void SetUnitTag(const FGameplayTag& InTag);

	virtual TArray<FGameplayTag> GetEquipItemTags() const override;
	void SetItemTag(const FGameplayTag& InItemTag);

	FGameplayTag GetEquipItemTag() const { return ItemTag;}

	// 캐러샐 유닛 부착

	UPROPERTY(Replicated)
	bool bPicked = false;

	UPROPERTY(Replicated)
	int32 PickedBySeat = INDEX_NONE;

	UPROPERTY()
	TWeakObjectPtr<APCCarouselRing> OwnerRing;

	bool IsPicked() const { return bPicked; }
	void MarkPicked();
	
	UFUNCTION(Server, Reliable)
	void Server_StartFollowing(APCPlayerCharacter* Picker);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachToCarrier(APCPlayerCharacter* Picker);

	UPROPERTY(Replicated)
	TWeakObjectPtr<APCPlayerCharacter> Carrier;

	UPROPERTY(EditDefaultsOnly)
	bool bDisableCollisionWhenCarried = true;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 캐러셀 유닛 픽업 이벤트
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ring")
	TWeakObjectPtr<APCCarouselRing> CarouselRing;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UWidgetComponent> StatusBarComp;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	FName StatusBarSocketName = TEXT("HealthBar");

	void InitCarouselAnimInstance() const;
	void InitStatusBar();
	void AttachStatusBarToSocket() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<const UPCHeroUnitAttributeSet> HeroAttributeSet;

	UPROPERTY(BlueprintReadOnly, Category="Data")
	TObjectPtr<const UPCDataAsset_HeroUnitData> HeroData = nullptr;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_UnitTag, Category="Data")
	FGameplayTag UnitTag;
	
	UFUNCTION()
	void OnRep_UnitTag();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ItemTag, Category="Data")
	FGameplayTag ItemTag;

	UFUNCTION()
	void OnRep_ItemTag();
};
