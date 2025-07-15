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
	PlayerClass = nullptr;
	PlayerCombatPawnRef = nullptr;
	EnemyClass = nullptr;
	EnemyCombatPawnRef = nullptr;
	AttackedFieldMonsterRef = nullptr;
	ReturnToFieldMapName = NAME_None;
}

void UMyGameInstance::StartBattleTransition(APlayerCharacter* PlayerActor, AMonsterCharacter* EnemyActor, FName CurrentMapName)
{
	if (!PlayerActor || !EnemyActor)
	{
		UE_LOG(LogTemp, Error, TEXT("StartBattleTransition: 유효하지 않은 Actor. 전투 시작 중단."));
		return;
	}

	ResetBattleData();

	PlayerClass = PlayerActor->GetClass();
	PlayerCombatPawnRef = PlayerActor;

	EnemyClass = EnemyActor->GetClass();
	EnemyCombatPawnRef = EnemyActor;

	ReturnToFieldMapName = CurrentMapName;
	AttackedFieldMonsterRef = EnemyActor;

	UGameplayStatics::OpenLevel(this, FName("BattleMap_01"), true);
}

void UMyGameInstance::ReturnToFieldTransition(bool bPlayerWon)
{
    // 필드 레벨로 돌아감
    UGameplayStatics::OpenLevel(this, ReturnToFieldMapName, true);
}
