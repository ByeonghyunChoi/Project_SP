// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatEncounterData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

public:
	// 스폰할 적 클래스 (예: BP_Goblin)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<class AActor> EnemyClass;

	// 스폰 위치 인덱스 (전투 맵에 미리 배치된 SpawnPoint의 번호)
	// 0=중앙, 1=좌측, 2=우측
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnPositionIndex;

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
