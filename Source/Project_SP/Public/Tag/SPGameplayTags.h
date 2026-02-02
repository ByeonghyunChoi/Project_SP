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
	// 상태(State)
	FGameplayTag State_Mode_Field;
	FGameplayTag State_Mode_Battle;
	FGameplayTag State_Turn_Active;
	FGameplayTag State_Status_BlockMove;

	// 입력(Input)
	FGameplayTag Input_Action_Interact;
	FGameplayTag Input_Action_FieldAttack;

	// 어빌리티 타입(Ability Type)
	FGameplayTag Ability_Type_Field;
	FGameplayTag Ability_Type_Battle;

	// 오파츠 (Oparts)
	FGameplayTag Oparts_CrystalSkull;
	FGameplayTag Oparts_JadeClock;
	FGameplayTag Oparts_GoldBug;

protected:
	//태그를 등록하는 내부 함수
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
