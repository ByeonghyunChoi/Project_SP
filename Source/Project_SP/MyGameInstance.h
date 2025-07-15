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

	// 전투 이전의 플레이어 객체
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APlayerCharacter> PlayerClass;

	// 전투 이전의 플레이어 상태 저장 (CombatPawn)
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ACombatPawn> PlayerCombatPawnRef;

	// 적 클래스 및 상태 저장
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AMonsterCharacter> EnemyClass;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ACombatPawn> EnemyCombatPawnRef;

	// 전투 후 돌아갈 필드 맵 이름
	UPROPERTY(BlueprintReadWrite)
	FName ReturnToFieldMapName;

	// 전투 후 처리할 대상 몬스터 (필드에 있는 원본)
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AMonsterCharacter> AttackedFieldMonsterRef;

	// 전투 데이터 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetBattleData();

	// 전투 시작을 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBattleTransition(APlayerCharacter* PlayerActor, AMonsterCharacter* EnemyActor, FName CurrentFieldName);

	// 전투 종료 후 필드 맵으로 돌아갈 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReturnToFieldTransition(bool bPlayerWon);
};
