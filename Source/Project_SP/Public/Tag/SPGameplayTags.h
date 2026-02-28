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

public:
	// 상태(State)
	FGameplayTag State_Mode_Field;
	FGameplayTag State_Mode_Battle;
	FGameplayTag State_Turn_Active;
	FGameplayTag State_Status_BlockMove;
	FGameplayTag State_Battle_TurnActive;

	// 입력(Input)
	FGameplayTag Input_Action_Interact;
	FGameplayTag Input_Action_FieldAttack;
	//테스트용 치트 버튼(나중에 제거)
	FGameplayTag Input_Action_CheatReturn;

	// 타입(Ability Type)
	FGameplayTag Ability_Type_Field;
	FGameplayTag Ability_Type_Battle;

	// 오파츠(Oparts)
	FGameplayTag Oparts_CrystalSkull;
	FGameplayTag Oparts_JadeClock;
	FGameplayTag Oparts_GoldBug;

	// 유물(Relic)
	FGameplayTag Relic_Bonus_Attack;

	//이벤트
	FGameplayTag Event_Interaction;
	FGameplayTag Event_Field_Hit;
	FGameplayTag Event_Battle_TurnStart;
	FGameplayTag Event_Battle_TurnEnd;

protected:
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
