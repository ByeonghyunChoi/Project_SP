// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMyGameInstance::Init()
{
	Super::Init();
}

void UMyGameInstance::StartBattleTransition(APlayerCharacter* PlayerActor, AMonsterCharacter* EnemyActor, AMonsterCharacter* AttackedMonster, FName CurrentFieldName)
{
    PlayerActorRef = PlayerActor;
    EnemyActorRef = EnemyActor;
    AttackedFieldMonsterActor = AttackedMonster; // 이 액터 레퍼런스는 다음 레벨 로드 시 유효하지 않을 수 있습니다.
    // 실제로는 이 몬스터의 고유 ID를 저장하여 필드 맵 복귀 시 찾아 제거하는 방식이 더 안전합니다.
    ReturnToFieldName = CurrentFieldName;

    // 전투 레벨 로드 (전투 레벨의 이름을 "BattleMap"이라고 가정)
    UGameplayStatics::OpenLevel(this, FName("BattleMap"), true); // true는 Seamless Travel (부드러운 전환)
}

void UMyGameInstance::ReturnToFieldTransition(bool bPlayerWon)
{
    // 전투 결과 처리 (예: 몬스터 제거, 플레이어 경험치 획득 등)
    // 이 로직은 주로 FieldGameMode의 BeginPlay나 PlayerCharacter의 BeginPlay에서 처리하는 것이 좋습니다.
    // GameInstance는 데이터를 전달하는 역할에 집중.

    // 필드 레벨로 돌아감
    UGameplayStatics::OpenLevel(this, ReturnToFieldName, true);
}
