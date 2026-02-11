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
	// --- A. 자원 (현재 값 저장) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Resource")
	float CurrentHealth = -1.0f; // -1: 초기화 안 됨

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Resource")
	float CurrentTimePower = 0.0f;

	// --- B. 기초 스탯 (Base Value 저장) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Basic")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Basic")
	float AttackPower = 10.0f;

	// --- C. 턴/전투 스탯 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Combat")
	float Speed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Combat")
	float CritChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Combat")
	float CritDamage = 0.5f;

	// --- D. 데미지 보정 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Mod")
	float DamageDealtInc = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Mod")
	float DamageDealtDec = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Mod")
	float DamageReceivedInc = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Mod")
	float DamageReceivedDec = 0.0f;

	// --- E. 방어/효과 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Utility")
	float Durability = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Utility")
	float IgnoreDurability = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Utility")
	float EffectEfficiency = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Utility")
	float EffectProbability = 0.0f;

	// --- F. 성장 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Growth")
	float Level = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData | Growth")
	float Experience = 0.0f;
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
