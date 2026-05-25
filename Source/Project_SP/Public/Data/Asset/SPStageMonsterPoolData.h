// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/Asset/SPMonsterData.h"
#include "SPStageMonsterPoolData.generated.h"


USTRUCT(BlueprintType)
struct FStageMonsterPool
{
	GENERATED_BODY()

	// 이 스테이지에서 전투가 벌어질 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CombatLevelName;

	// 이 스테이지에 등장할 일반 몬스터들 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<USPMonsterData*> NormalMonsters;

	// 이 스테이지에 등장할 강적 몬스터들 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<USPMonsterData*> StrongMonsters;
};

UCLASS()
class PROJECT_SP_API USPStageMonsterPoolData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnPool")
	TMap<int32, FStageMonsterPool> StagePools;
};
