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
	State_Status_BlockMove = Manager.AddNativeGameplayTag(FName("State.Status.BlockMove"), TEXT("이동 불가 상태"));
	State_Battle_TurnActive = Manager.AddNativeGameplayTag(FName("State.Battle.TurnActive"), TEXT("턴 활성화 상태"));

	Input_Action_Interact = Manager.AddNativeGameplayTag(FName("Input.Action.Interact"), TEXT("상호작용 입력"));
	Input_Action_FieldAttack = Manager.AddNativeGameplayTag(FName("Input.Action.FieldAttack"), TEXT("필드 공격 입력"));
	Input_Action_CheatReturn = Manager.AddNativeGameplayTag(FName("Input.Action.CheatReturn"), TEXT("전투 복귀 치트 입력"));

	Ability_Type_Field = Manager.AddNativeGameplayTag(FName("Ability.Type.Field"), TEXT("필드 타입 어빌리티"));
	Ability_Type_Battle = Manager.AddNativeGameplayTag(FName("Ability.Type.Battle"), TEXT("전투 타입 어빌리티"));

	Oparts_CrystalSkull = Manager.AddNativeGameplayTag(FName("Oparts.CrystalSkull"), TEXT("오파츠: 수정 해골"));
	Oparts_JadeClock = Manager.AddNativeGameplayTag(FName("Oparts.JadeClock"), TEXT("오파츠: 옥시계"));
	Oparts_GoldBug = Manager.AddNativeGameplayTag(FName("Oparts.GoldBug"), TEXT("오파츠: 골드버그"));
	
	Event_Interaction = Manager.AddNativeGameplayTag(FName("Event.Interaction"), TEXT("상호작용 이벤트 활성화"));
	Event_Field_Hit = Manager.AddNativeGameplayTag(FName("Event.Field.Hit"), TEXT("필드공격 이벤트 활성화"));
	Event_Battle_TurnStart = Manager.AddNativeGameplayTag(FName("Event.Battle.TurnStart"), TEXT("턴 시작 시 이벤트 활성화"));
	Event_Battle_TurnEnd = Manager.AddNativeGameplayTag(FName("Event.Battle.TurnEnd"), TEXT("턴 종료 시 이벤트 활성화"));

}