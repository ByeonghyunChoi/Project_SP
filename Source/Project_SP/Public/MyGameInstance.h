// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerCharacter.h"
#include "MonsterCharacter.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECT_SP_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	//플레이어 스탯 데이터
	UPROPERTY(BlueprintReadWrite, Category = "CombatData")
	UCharacterStatsComponent* StoredPlayerStatsComponent;

	//몬스터 클래스 정보
	UPROPERTY(BlueprintReadWrite)
	UMonsterGroupObject* PendingMonsterGroup;

	// 전투 후 돌아갈 필드 맵 이름
	UPROPERTY(BlueprintReadWrite)
	FName ReturnToFieldMapName;

	// 전투 데이터 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetBattleData();

	// 전투 시작을 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBattleTransitionWithGroup(APlayerCharacter* PlayerActor, UMonsterGroupObject* MonsterGroup);

	// 전투 종료 후 필드 맵으로 돌아갈 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReturnToFieldTransition(bool bPlayerWon);
};
