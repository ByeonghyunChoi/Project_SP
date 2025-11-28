#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Combat/CombatTypes.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Blueprint/UserWidget.h"
#include "MapManagerSubsystem.generated.h"

// 전방 선언
class UMapNode;
class UMapGraphGenerator;
class AMapBase;

/**
 * @brief 맵 데이터 테이블 구조체
 * 레벨 파일(배경)과 로직 클래스(기능)를 분리하여 정의합니다.
 */
USTRUCT(BlueprintType)
struct FMapDataRow : public FTableRowBase
{
	GENERATED_BODY()

	// 1. 배경이 되는 레벨 에셋 (.umap)
	// TSoftObjectPtr를 사용하여 필요할 때만 로딩합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LevelAsset;

	// 2. 해당 맵의 규칙을 담당할 로직 클래스 (BP_Map_Normal 등)
	// 레벨 로딩 후 이 클래스가 동적으로 스폰됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AMapBase> MapLogicClass;
};

UCLASS()
class PROJECT_SP_API UMapManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- 외부 호출 함수 ---

	// [진입점] 포탈 이동 시 호출. 화면 암전 후 로딩 프로세스를 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void TravelToNode(UMapNode* TargetNode);

	// 게임 시작 (1스테이지 로드)
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void StartNewRun();

	// 다음 스테이지(층)로 이동
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void GoToNextStage();

	// 전투 승리 알림 (BattleTransitionManager -> MapManager -> MapBase)
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void NotifyCombatFinished(bool bPlayerWon);

	// 허브(로비)로 귀환
	UFUNCTION(BlueprintCallable, Category = "Map Manager")
	void ReturnToHub(bool bPlayerWon);

protected:
	// 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MapTypeData;

	// 현재 로드된 스트리밍 레벨 인스턴스 (언로드를 위해 저장)
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> CurrentLevelInstance;

	// 현재 맵의 로직을 담당하는 액터 (AMapBase)
	UPROPERTY()
	TObjectPtr<AMapBase> CurrentMapLogicActor;

	// 화면 전환 위젯 클래스 및 인스턴스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TransitionWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentTransitionWidget;

	// 이동할 목표 노드 임시 저장
	UPROPERTY()
	TObjectPtr<UMapNode> PendingNode;

	// 맵 그래프 생성기
	UPROPERTY()
	TObjectPtr<UMapGraphGenerator> MapGenerator;

	// 맵 진행 상태
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Map State")
	int32 CurrentStage = 1;
	int32 MaxStages = 3;

	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TObjectPtr<UMapNode> GraphRoot;

	UPROPERTY(VisibleInstanceOnly, Category = "Map State")
	TObjectPtr<UMapNode> CurrentNode;

	FName HubSpawnPointTag = TEXT("HubStart");

private:
	// --- 내부 로딩 프로세스 (순서대로 실행됨) ---

	// 1. Fade In(암전) 완료 콜백 -> 언로드 시작
	UFUNCTION()
	void OnFadeInFinished();

	// 2. 기존 레벨 언로드
	void UnloadPreviousLevel();

	// 2-1. 언로드 완료 콜백 -> 새 레벨 로드 시작
	UFUNCTION()
	void OnLevelUnloaded();

	// 3. 새 레벨 비동기 로드
	void LoadNextLevel();

	// 3-1. 로드 완료 콜백 -> 맵 초기화 및 화면 밝기
	UFUNCTION()
	void OnLevelLoaded();

	// 4. Fade Out(화면 밝아짐) 실행
	void PerformFadeOut();

	void GenerateNewStageGraph();
};