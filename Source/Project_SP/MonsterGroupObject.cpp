// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterGroupObject.h"

void UMonsterGroupObject::CopyMonterGroup(UMonsterGroupObject* OtherGroup)
{
	MonsterClasses = OtherGroup->MonsterClasses;
}
