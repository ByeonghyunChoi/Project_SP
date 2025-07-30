// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterData.generated.h"

class AMonsterCharacter;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMonsterData : public FTableRowBase
{
	GENERATED_BODY()

public:
	//몬스터 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	FText DisplayName;

	//몬스터 클래스 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	TSubclassOf<class AMonsterCharacter> MonsterClass;

	//몬스터 스탯 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	FName CharacterStatsRowName;

	//몬스터가 사용할 행동 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	TArray<FName> AvailableActionIDs;
};

