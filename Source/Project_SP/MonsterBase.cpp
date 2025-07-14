// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterBase.h"

UMonsterBase::UMonsterBase()
{
	SetFaction(EFaction::Enemy);
}

void UMonsterBase::DecideAction()
{
	// 다형성을 위한 AI로직 기본은 일반 공격

}
