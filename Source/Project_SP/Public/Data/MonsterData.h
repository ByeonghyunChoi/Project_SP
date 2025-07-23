// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CharacterStatsData.h"
#include "MonsterData.generated.h"

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

	//몬스터의 AI에 대한 참조 
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	// class UBehaviorTree* MonsterBehaviorTree;

	// 몬스터가 드랍할 아이템 테이블 ID(나중에 추가)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Info")
	// FName ItemDropTableID;
};

