// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/MonsterGroupObject.h"

void UMyGameInstance::Init()
{
	Super::Init();
    ResetBattleData();
}


void UMyGameInstance::ResetBattleData()
{
	if (PendingMonsterGroup)
	{
		PendingMonsterGroup->ConditionalBeginDestroy();
		PendingMonsterGroup = nullptr;
	}

	ReturnToFieldMapName = NAME_None;

}

void UMyGameInstance::StartBattleTransitionWithGroup(UMonsterGroupObject* MonsterGroup, FName MapName)
{
    PendingMonsterGroup = MonsterGroup; // 그룹 데이터 저장 (주로 BP에서 생성된 UMonsterGroupObject 인스턴스)
    ReturnToFieldMapName = MapName;

    UGameplayStatics::OpenLevel(this, FName("BattleMap_01")); // 전투 맵 이름으로 변경
}

void UMyGameInstance::ReturnToFieldTransition()
{
    if (ReturnToFieldMapName != NAME_None)
    {
        UGameplayStatics::OpenLevel(this, ReturnToFieldMapName);
    }
    ResetBattleData(); // 데이터 클리어
}
