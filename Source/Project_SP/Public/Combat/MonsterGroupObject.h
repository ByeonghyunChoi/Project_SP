// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/MonsterData.h"
#include "MonsterGroupObject.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECT_SP_API UMonsterGroupObject : public UObject
{
	GENERATED_BODY()

public:
	//사용할 몬스터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Group")
	UDataTable* MonsterDataTable;

	//참조해서 가져올 몬스터 RowName 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Group")
	TArray<FName> MonsterIDsInGroup;

	// 필요하다면, 그룹의 모든 몬스터 데이터를 한 번에 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Monster Group")
	TArray<FMonsterData> GetAllMonsterDataInGroup() const;

	// 필요하다면, MonsterID에 해당하는 FMonsterData를 반환하는 함수
	UFUNCTION(BlueprintPure, Category = "Monster Group")
	FMonsterData GetMonsterData(FName MonsterID) const;
	
	// 다른 몬스터 그룹의 정보를 복사하는 함수 (필요시 업데이트)
	void CopyMonterGroup(UMonsterGroupObject* OtherGroup);
};
