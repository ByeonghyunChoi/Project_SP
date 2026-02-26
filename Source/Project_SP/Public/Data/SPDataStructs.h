// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPDataStructs.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlayerStatsData
{
	GENERATED_BODY()

public:
    UPROPERTY() float CurrentHealth = 100.0f;
    UPROPERTY() float CurrentBattlePoint = 0.0f;
    UPROPERTY() float CurrentTimePower = 200.0f;    
    UPROPERTY() float CurrentActionGauge = 0.0f;

    UPROPERTY() float MaxHealth = 100.0f;
    UPROPERTY() float MaxBattlePoint = 5.0f;
    UPROPERTY() float MaxTimePower = 9999.0f;       

    UPROPERTY() float Attack = 20.0f;
    UPROPERTY() float Defense = 10.0f;
    UPROPERTY() float Speed = 100.0f;               

    // 3. 전투 보조 스탯
    UPROPERTY() float DefenseIgnore = 0.0f;
    UPROPERTY() float CriticalRate = 0.05f;
    UPROPERTY() float CriticalDamage = 1.5f;
    UPROPERTY() float EffectHitRate = 0.0f;
    UPROPERTY() float EffectAmplify = 0.0f;

    // 4. 배율
    UPROPERTY() float OutgoingDamageMultiplier = 1.0f;
    UPROPERTY() float IncomingDamageMultiplier = 1.0f;

    // 5. 성장
    UPROPERTY() float Level = 1.0f;
    UPROPERTY() float Experience = 0.0f;
    UPROPERTY() float MaxExperience = 100.0f;

    // 6. 속성 저항
    UPROPERTY() float ResistanceSurtr = 0.0f;
    UPROPERTY() float ResistanceFenrir = 0.0f;
    UPROPERTY() float ResistanceJormungandr = 0.0f;
};

USTRUCT(BlueprintType)
struct FPlayerPersistentData
{
	GENERATED_BODY()

	// 스탯 데이터 포함 (계층 구조)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerStatsData Stats;

	// (나중에 Inventory, WorldProgress 등이 여기에 추가됨)

	// 유효성 검사 (체력이 0 이상이면 저장된 데이터가 있다고 판단)
	bool IsValid() const { return Stats.CurrentHealth >= 0.0f; }

	// 초기화
	void Reset()
	{
		Stats = FPlayerStatsData(); // 기본값으로 리셋
	}
};
