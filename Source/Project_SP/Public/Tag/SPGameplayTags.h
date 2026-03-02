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
	FGameplayTag State_Cooldown;
	FGameplayTag State_Status_SkipTurn;
	FGameplayTag State_TimeInterference;

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
	FGameplayTag Event_Montage_Hit;
	FGameplayTag Event_Hit_Start;
	FGameplayTag Event_Hit_Bounce;
	FGameplayTag Event_Hit_AoE;

	// 무기(Weapon)
	FGameplayTag Weapon_Fenrir;
	FGameplayTag Weapon_Surtr;
	FGameplayTag Weapon_Jormungandr;

	//약점(Weakness)
	FGameplayTag Weakness_Fenrir;
	FGameplayTag Weakness_Surtr;
	FGameplayTag Weakness_Jormungandr;

	//쿨타임(Cooldown)
	FGameplayTag Cooldown_Weapon_Fenrir_Skill;
	FGameplayTag Cooldown_Weapon_Fenrir_Parry;
	FGameplayTag Cooldown_Weapon_Surtr_Skill;
	FGameplayTag Cooldown_Weapon_Surtr_Parry;
	FGameplayTag Cooldown_Weapon_Jormungandr_Skill;
	FGameplayTag Cooldown_Weapon_Jormungandr_Parry;
	FGameplayTag Cooldown_Skill_TimeInterference;

	//상태 이상(Debuff)
	FGameplayTag Debuff_Basic_Weathering;  // 풍화
	FGameplayTag Debuff_Basic_Burn;        // 화상
	FGameplayTag Debuff_Basic_Poison;      // 중독
	FGameplayTag Debuff_Mix_HeatWind;      // 열풍
	FGameplayTag Debuff_Mix_Plague;        // 역병
	FGameplayTag Debuff_Mix_Faint;         // 혼절
	FGameplayTag Debuff_Fatal_FatalWound;  // 치명상

	// 전투 행동(Battle.Action)
	FGameplayTag Battle_Action_Attack;
	FGameplayTag Battle_Action_Skill;
	FGameplayTag Battle_Action_Parry;
	FGameplayTag Battle_Action_TimeInterference;

	//데미지 종류
	FGameplayTag Damage_Type_Normal;
	FGameplayTag Damage_Type_Status; 
	FGameplayTag Damage_Type_Fixed;
	FGameplayTag Damage_Type_Execute;
	
	//몬스터 등급
	FGameplayTag Enemy_Rank_Normal;
	FGameplayTag Enemy_Rank_Epic;
	FGameplayTag Enemy_Rank_Boss;

	//데이터 전달(Data)
	FGameplayTag Data_Damage;

protected:
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
