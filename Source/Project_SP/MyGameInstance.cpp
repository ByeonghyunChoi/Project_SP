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
    PlayerCharacterClassToSpawn = PlayerActor->GetClass();
    EnemyCharacterClassToSpawn = EnemyActor->GetClass();

    // 전투 데이터 복사 (FCharacterStatsData는 USTRUCT이므로 직접 복사 가능)
    PlayerPersistedStats = PlayerActor->GetCombatData()->GetStats();
    EnemyPersistedStats = EnemyActor->GetCombatData()->GetStats();

    // 필드 몬스터 액터 레퍼런스 저장 (필드 복귀 시 사용)
    AttackedFieldMonsterActor = EnemyActor;

    ReturnToFieldName = CurrentFieldName;

    UE_LOG(LogTemp, Log, TEXT("UMyGameInstance: 전투 데이터 저장 완료. BattleMap으로 전환 시작."));
    UGameplayStatics::OpenLevel(this, FName("BattleMap_01"), true);
}

void UMyGameInstance::ReturnToFieldTransition(bool bPlayerWon)
{
    // 전투 결과 처리 (예: 몬스터 제거, 플레이어 경험치 획득 등)
    // 이 로직은 주로 FieldGameMode의 BeginPlay나 PlayerCharacter의 BeginPlay에서 처리하는 것이 좋습니다.
    // GameInstance는 데이터를 전달하는 역할에 집중.

    // 필드 레벨로 돌아감
    UGameplayStatics::OpenLevel(this, ReturnToFieldName, true);
}
