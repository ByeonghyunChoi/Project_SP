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
	FGameplayTag State_Death;
	FGameplayTag State_ParryWindow;
	FGameplayTag State_CounterMode;
	FGameplayTag State_AutoCounterReady;
	FGameplayTag State_ActionExecuting;

	// 입력(Input)
	FGameplayTag Input_Action_Interact;
	FGameplayTag Input_Action_FieldAttack;

	// 타입(Ability Type)
	FGameplayTag Ability_Type_Field;
	FGameplayTag Ability_Type_Battle;

	// 오파츠(Oparts)
	FGameplayTag Oparts_CrystalSkull;
	FGameplayTag Oparts_CrystalSkull_Artifact2;
	FGameplayTag Oparts_JadeClock;
	FGameplayTag Oparts_GoldBug;

	// 유물(Relic)
	FGameplayTag Relic_Bonus_Attack;
	FGameplayTag Relic_WornWhetstone; // 마모된 숫돌
	FGameplayTag Relic_FadingCandle; // 꺼져가는 양초
	FGameplayTag Relic_RustedNeedle; // 녹슨 구리 침
	FGameplayTag Relic_Passive_ResonatingRune; // 공명하는 룬


	//이벤트
	FGameplayTag Event_Interaction;
	FGameplayTag Event_Field_Hit;
	FGameplayTag Event_Battle_TurnStart;
	FGameplayTag Event_Battle_TurnEnd;
	FGameplayTag Event_Battle_ExecuteAction;
	FGameplayTag Event_Battle_ApplyDamage;
	FGameplayTag Event_Battle_Parried;
	FGameplayTag Event_Montage_Hit;
	FGameplayTag Event_Hit_Start;
	FGameplayTag Event_Hit_Bounce;
	FGameplayTag Event_Hit_AoE;
	FGameplayTag Event_Jormungandr_Fire;
	FGameplayTag Event_Jourmungandr_Hit;

	//유물 전용 이벤트들
	FGameplayTag Event_Combat_CriticalHit;
	FGameplayTag Event_Battle_Start;
	FGameplayTag Event_Battle_End;
	FGameplayTag Event_Combat_AttackHit;
	FGameplayTag Event_Combat_AttackKill;
	FGameplayTag Event_Combat_SkillKill;
	FGameplayTag Event_Combat_ParrySuccess;

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
	FGameplayTag Battle_Action_CounterAttack;
	FGameplayTag Battle_Action_TimeInterference;
	FGameplayTag Battle_Monster_BasicAttack; // 임시 몬스터 공격 태그

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
	FGameplayTag Data_HealAmount; // 힐량 전달

	//이펙트, 사운드
	FGameplayTag GameplayCue_Hit_Fenrir;
	FGameplayTag GameplayCue_Hit_Surtr;
	FGameplayTag GameplayCue_Hit_Jormungandr;
	FGameplayTag GameplayCue_Hit_Player;
	FGameplayTag GameplayCue_Parry;
protected:
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
