// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMyGameInstance::Init()
{
	Super::Init();
    ResetBattleData();
}


void UMyGameInstance::ResetBattleData()
{
	PendingMonsterGroup = nullptr;
	ReturnToFieldMapName = NAME_None;
	StoredPlayerStatsComponent = nullptr;
}

void UMyGameInstance::StartBattleTransitionWithGroup(APlayerCharacter* PlayerActor, UMonsterGroupObject* MonsterGroup)
{
	if (!PlayerActor || !MonsterGroup)
	{
		UE_LOG(LogTemp, Error, TEXT("StartBattleTransitionWithGroup: 유효하지 않은 파라미터. 전투 시작 중단."));
		return;
	}

	ResetBattleData();
	UCharacterStatsComponent* PlayerActorStats = PlayerActor->FindComponentByClass<UCharacterStatsComponent>();
	if (PlayerActorStats)
	{
		if(!StoredPlayerStatsComponent) // 기존에 인스턴스가 없으면 새로 생성
		{
			StoredPlayerStatsComponent = NewObject<UCharacterStatsComponent>();
			StoredPlayerStatsComponent->CopyFrom(PlayerActorStats);
		}
	}

	PendingMonsterGroup = NewObject<UMonsterGroupObject>();
	PendingMonsterGroup->CopyMonterGroup(MonsterGroup);

	ReturnToFieldMapName = FName("FieldMap");

	UGameplayStatics::OpenLevel(this, FName("BattleMap_01"), true);
}

void UMyGameInstance::ReturnToFieldTransition(bool bPlayerWon)
{
    // 필드 레벨로 돌아감
    UGameplayStatics::OpenLevel(this, ReturnToFieldMapName, true);
}
