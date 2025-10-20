// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"


const FPCProjectileData* UPCDataAsset_ProjectileData::GetProjectileData(FGameplayTag Tag) const
{
	return ProjectileData.Find(Tag);
}
