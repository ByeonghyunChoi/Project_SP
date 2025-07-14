// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMyGameInstance::Init()
{
	Super::Init();
}


void UMyGameInstance::StartBattleTransition(APlayerCharacter* PlayerActor, AMonsterCharacter* EnemyActor, FName CurrentFieldName)
{
    // 클래스 저장 (스폰용)
    EnemyCharacterClassToSpawn = EnemyActor->GetClass();

    // 전투 데이터 복사
    PlayerPersistedStats = NewObject<UCharacterStats>(this, UCharacterStats::StaticClass());
    PlayerPersistedStats->CopyFrom(PlayerActor->GetCombatData()->GetStats());

    EnemyPersistedStats = NewObject<UCharacterStats>(this, UCharacterStats::StaticClass());
    EnemyPersistedStats->CopyFrom(EnemyActor->GetCombatData()->GetStats());

    // 필드 몬스터 액터 레퍼런스 저장 (필드 복귀 시 사용)
    AttackedFieldMonsterActor = EnemyActor;

    ReturnToFieldName = CurrentFieldName;

    UE_LOG(LogTemp, Log, TEXT("UMyGameInstance: 전투 데이터 저장 완료. BattleMap으로 전환 시작."));
    UGameplayStatics::OpenLevel(this, FName("BattleMap_01"), true);
}

void UMyGameInstance::ReturnToFieldTransition(bool bPlayerWon)
{
    // 필드 레벨로 돌아감
    UGameplayStatics::OpenLevel(this, ReturnToFieldName, true);
}
