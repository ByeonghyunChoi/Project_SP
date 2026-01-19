// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 
 */
struct FSPGameplayTags
{
public:
	static const FSPGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	//등록할 태그 목록 섹션
public:
	FGameplayTag State_Mode_Field;
	FGameplayTag State_Mode_Battle;
	FGameplayTag State_Turn_Active;

protected:
	//태그를 등록하는 내부 함수
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
