#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapInfo.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "MapManagerSubsystem.generated.h"

// 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FMapLevelData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LevelReference;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> BackgroundMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EMapType, TSubclassOf<class AMapBase>> MapClasses;
};

UCLASS()
class PROJECT_SP_API UMapManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// 게임 시작
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartNewRun();

	// 포탈을 탔을 때 호출 (다음 층 또는 다음 스테이지 이동)
	void MoveToNextFloor(EMapType SelectedType);

	// 다음 층의 선택지 생성 (UI 표시용)
	TArray<EMapType> GenerateNextFloorOptions();

	// 전투 맵 진입 (별도 레벨 스트리밍 시)
	void EnterBattle(TSoftObjectPtr<UWorld> BattleLevelRes);
	void ExitBattle();

	// 새 레벨이 열릴 때 MapBase가 호출해주는 함수
	void InitializeCurrentMap(class AMapBase* InMapActor);

	//로비로 이동하는 함수
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void GoToLobby();

protected:
	//로비 레퍼런스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LobbyLevelReference;

protected:
	EMapGrade GetMapGradeByFloor(int32 Floor) const;
	EMapType GetRandomTypeFromGrade(EMapGrade Grade) const;
	void LoadStageLevel();

	UFUNCTION()
	void OnBattleLevelLoaded();


private:
	int32 CurrentStage = 1;
	int32 CurrentFloor = 1;

	UPROPERTY()
	FTransform SavedFieldTransform;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> ActiveBattleLevel;

	UPROPERTY()
	TObjectPtr<class AMapBase> CurrentMapActor;

	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	const FVector BattleMapOffset = FVector(0.f, 0.f, -50000.f);

private:
	void SpawnMapActor(EMapType MapType);

	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
};