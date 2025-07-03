// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CharacterBase.h"
#include "MonsterCharacter.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	//전투 시작 시 전달될 플레이어 인스턴스
	UPROPERTY(BlueprintReadWrite, Category = "BattleData")
	UCharacterBase* PlayerCombatantRef;

	// 전투 시작 시 전달될 적 UCharacterBase 인스턴스
	UPROPERTY(BlueprintReadWrite, Category = "BattleData")
	UCharacterBase* EnemyCombatantRef;

	// 필드 맵에서 공격한 몬스터 액터의 레퍼런스 (전투 종료 후 제거용)
	// 주의: 액터 레퍼런스는 레벨 로드 후 유효하지 않을 수 있으므로, 액터 ID나 이름을 저장하는 것이 더 안전합니다.
	// 여기서는 예시로 사용하며, 실제 구현에서는 더 견고한 방법(예: Unique ID 시스템) 고려
	UPROPERTY(BlueprintReadWrite, Category = "BattleData")
	AMonsterCharacter* AttackedFieldMonsterActor;

	// 전투 종료 후 돌아갈 필드 맵 이름
	UPROPERTY(BlueprintReadWrite, Category = "BattleData")
	FName ReturnToFieldName;

	// 전투 시작을 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBattleTransition(UCharacterBase* PlayerCharBase, UCharacterBase* EnemyCharBase, AMonsterCharacter* AttackedMonster, FName CurrentFieldName);

	// 전투 종료 후 필드 맵으로 돌아갈 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReturnToFieldTransition(bool bPlayerWon);
};
