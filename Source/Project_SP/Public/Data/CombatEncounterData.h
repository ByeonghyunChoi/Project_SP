// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/Asset/SPMonsterData.h"
#include "CombatEncounterData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

public:
	// 스폰할 적 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	class USPMonsterData* MonsterData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpawnLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Override")
	bool bOverrideWeakness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Override", meta = (EditCondition = "bOverrideWeakness"))
	FGameplayTagContainer OverriddenWeaknessTags;

	// 스폰 위치 인덱스 0=중앙, 1=좌측, 2=우측
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnPositionIndex = 0;

	

};

UCLASS()
class PROJECT_SP_API UCombatEncounterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 이 전투에 등장할 적들의 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TArray<FEnemySpawnInfo> EnemyGroup;

	// 전투가 벌어질 레벨의 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	FName CombatLevelName;
	
};
