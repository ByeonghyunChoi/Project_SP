// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Combat/CombatTypes.h"
#include "Engine/DataTable.h"
#include "MapManagerSubsystem.generated.h"

/**
 * 
 */

//맵 데이터 구조체
USTRUCT(BlueprintType)
struct FMapDataRow : public FTableRowBase
{
	GENERATED_BODY()

	/* 맵 타입의 종류에 맞게 스폰될 AMapBase의 자식 클래스 (예: BP_NormalMap, BP_RestMap) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AMapBase> MapClass;
};

class UMapNode;
class UMapGraphGenerator;
class AMapBase;

UCLASS()
class PROJECT_SP_API UMapManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 1스테이지 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void StartNewRun();

	//플레이어가 죽거나 마지막 스테이지를 클리어하면 게임 시작 맵으로 돌아가는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void ReturnToHub(bool bPlayerWon);

	//다음 맵으로 이동하는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void TravelToNode(UMapNode* TargetNode);

	//보스 클리어 후 다음 스테이지로 이동하는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void GoToNextStage();

protected:
	//로드된 데이터 테이블을 저장할 실제 포인터
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> MapTypeData;

	//맵 그래프 생성기 인스턴스
	UPROPERTY()
	TObjectPtr<UMapGraphGenerator> MapGenerator;

	//현재 스테이지
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Map State")
	int32 CurrentStage = 1;

	//현재 스테이지의 전체 맵 그래프
	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TObjectPtr<UMapNode> GraphRoot;

	//플레이어가 현재 위치한 노드
	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TObjectPtr<UMapNode> CurrentNode;

	//현재 맵 액터
	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TObjectPtr<AMapBase> CurrentMapActorInstance;

	//클리어한 맵 노드 ID
	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TSet<FGuid> ClearedNodeIDs;

private:
	//새 스테이지 그래프를 생성하고 루트를 설정하는 함수
	void GenerateNewStageGraph();
	//게임 시작 맵(레벨) 이름
	FName HubSpawnPointTag;
	//스테이지 수
	int32 MaxStages; 
	//로그라이크 맵 스폰 위치
	FName DungeonSpawnPointTag;
};
