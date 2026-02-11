#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapInfo.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "SubSystem/SPCombatSubsystem.h"
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

	// 전투 맵 진입 
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartBattleEncounter(APawn* PlayerPawn, const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage);

	// 전투 종료 후 필드 복귀 
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ReturnToField();

	// 새 레벨이 열릴 때 MapBase가 호출해주는 함수
	void InitializeCurrentMap(class AMapBase* InMapActor);

	//로비로 이동하는 함수
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void GoToLobby();

	//현재 전투 맵에서 전투 중인지 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "Map")
	bool IsInBattleMap() const { return bIsBattleActive; }

protected:
	//로비 레퍼런스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LobbyLevelReference;

protected:
	EMapGrade GetMapGradeByFloor(int32 Floor) const;
	EMapType GetRandomTypeFromGrade(EMapGrade Grade) const;
	void LoadStageLevel();

private:
	int32 CurrentStage = 1;
	int32 CurrentFloor = 1;

	UPROPERTY()
	FTransform SavedFieldTransform;

	UPROPERTY()
	TObjectPtr<class AMapBase> CurrentMapActor;

	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	// 전투 복귀 체크용 플래그
	bool bIsReturningFromBattle = false;

	// 현재 전투중인지 확인하는 함수
	bool bIsBattleActive = false;

	// 전투 진입전 맵의 타입 저장용 복귀할 때 사용
	UPROPERTY(VisibleAnywhere, Category = "Debug") 
	EMapType CurrentMapType = EMapType::NormalBattle;

private:
	void SpawnMapActor(EMapType MapType);

	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
};