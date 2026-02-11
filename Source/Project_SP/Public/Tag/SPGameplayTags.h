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

	//����� �±� ��� ����
public:
	// ����(State)
	FGameplayTag State_Mode_Field;
	FGameplayTag State_Mode_Battle;
	FGameplayTag State_Turn_Active;
	FGameplayTag State_Status_BlockMove;

	// �Է�(Input)
	FGameplayTag Input_Action_Interact;
	FGameplayTag Input_Action_FieldAttack;

	// �����Ƽ Ÿ��(Ability Type)
	FGameplayTag Ability_Type_Field;
	FGameplayTag Ability_Type_Battle;

	// 오파츠 (Oparts)
	FGameplayTag Oparts_CrystalSkull;
	FGameplayTag Oparts_JadeClock;
	FGameplayTag Oparts_GoldBug;
	//�̺�Ʈ
	FGameplayTag Event_Interaction;
	FGameplayTag Event_Field_Hit;

protected:
	//�±׸� ����ϴ� ���� �Լ�
	void AddAllTags(class UGameplayTagsManager& Manager);

private:
	static FSPGameplayTags GameplayTags;
};
