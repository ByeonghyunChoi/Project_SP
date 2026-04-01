// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetManager/SPGASAssetManager.h"
#include "Tag/SPGameplayTags.h"

USPGASAssetManager& USPGASAssetManager::Get()
{
	check(GEngine);

	USPGASAssetManager* Singleton = Cast<USPGASAssetManager>(GEngine->AssetManager);
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogTemp, Fatal, TEXT("올바르지 않은 AssetManager입니다! 프로젝트 세팅(DefaultEngine.ini)에서 Asset Manager Class를 USPGASAssetManager로 변경하세요."));
	
	return *NewObject<USPGASAssetManager>();
}

void USPGASAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FSPGameplayTags::InitializeNativeGameplayTags();
}


