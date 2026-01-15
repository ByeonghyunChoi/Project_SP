// Fill out your copyright notice in the Description page of Project Settings.


#include "Tag/SPGameplayTags.h"
#include "GameplayTagsManager.h"

FSPGameplayTags FSPGameplayTags::GameplayTags;


void FSPGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.AddAllTags(UGameplayTagsManager::Get());
}

void FSPGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	State_Mode_Field = Manager.AddNativeGameplayTag(FName("State.Mode.Field"), TEXT("필드 상태"));
	State_Mode_Battle = Manager.AddNativeGameplayTag(FName("State.Mode.Battle"), TEXT("전투 상태"));
}