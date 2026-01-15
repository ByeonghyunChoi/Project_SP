// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetManager/SPGASAssetManager.h"
#include "Tag/SPGameplayTags.h"

USPGASAssetManager& USPGASAssetManager::Get()
{
	check(GEngine);

	if (USPGASAssetManager* Singleton = Cast<USPGASAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	return *NewObject<USPGASAssetManager>();
}

void USPGASAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FSPGameplayTags::InitializeNativeGameplayTags();
}


