#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapInfo.h"       // EMapType, EMapGrade 등 정의
#include "Engine/DataTable.h"
#include "Data/CombatEncounterData.h"
#include "SubSystem/SPCombatSubsystem.h" // ECombatAdvantage 정의
#include "MapManagerSubsystem.generated.h"

// 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FMapLevelData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LevelReference;

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

	// 새 게임 시작 
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartNewRun();

	// 전투 맵 진입 (필드 -> 전투)
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartBattleEncounter(APawn* PlayerPawn, const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage);

	// 전투 종료 후 필드 복귀 (전투 -> 필드)
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ReturnToField(bool bIsVictory);

	// 로비로 이동
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void GoToLobby();

	// 현재 맵 정보 초기화 (MapBase가 호출)
	void InitializeCurrentMap(class AMapBase* InMapActor);

	// 층 이동 로직
	void MoveToNextFloor(EMapType SelectedType);
	TArray<EMapType> GenerateNextFloorOptions();

	bool IsInBattleMap() { return bIsBattleActive; }
	
	//로비 맵 판독용
	bool GetIsInLobby() const { return bIsInLobby; }

	//Getter
	int32 GetCurrentStage() const { return CurrentStage; }
	int32 GetCurrentFloor() const { return CurrentFloor; }
	EMapType GetCurrentMapType() const { return CurrentMapType; }
	EMapState GetCurrentRoomState() const { return CurrentRoomState; }
	TArray<EMapType> GetCurrentPortalOptions() const { return CurrentPortalOptions; }
	//Setter
	void SetCurrentRoomState(EMapState NewState) { CurrentRoomState = NewState; }

	// 로드 시스템
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ResumeRunFromSave(int32 SavedStage, int32 SavedFloor, EMapType SavedMapType, EMapState SavedRoomState, FTransform SavedTransform, bool bSavedInLobby, TArray<EMapType> SavedPortalOptions);

protected:
	// 로비 레벨 레퍼런스 (에디터에서 경로 확인 필요)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LobbyLevelReference;

	// 맵 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

private:
	// 현재 스테이지 및 층 정보
	int32 CurrentStage = 1;
	int32 CurrentFloor = 1;

	// 현재 맵 타입 (전투 복귀 시 복구용)
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	EMapType CurrentMapType = EMapType::NormalBattle;

	// 현재 생성된 맵 액터
	TWeakObjectPtr<class AMapBase> CurrentMapActor;

	// [저장용] 필드 위치 저장
	UPROPERTY()
	FTransform SavedFieldTransform;

	// [저장용] 현재 포탈 연결 목록
	UPROPERTY()
	TArray<EMapType> CurrentPortalOptions;

	// [플래그] 전투에서 돌아오는 중인가?
	bool bIsReturningFromBattle = false;

	// [플래그] 현재 전투 중인가?
	bool bIsBattleActive = false;

	// [플래그] 현재 방이 클리어된 상태인가?
	EMapState CurrentRoomState = EMapState::InProgress;

	// [플래그] 세이브 파일에서 로드하여 맵에 진입하는 중인가?
	bool bIsLoadingSave = false;

	// [플래그] 플레이어가 로비 맵에 있는가?
	bool bIsInLobby = true;

private:
	// 맵 생성 및 플레이어 이동 처리
	void SpawnMapActor(EMapType MapType);

	// 레벨 로드 완료 시 호출되는 콜백 (위치 복구 핵심)
	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

	// 스테이지 레벨 로드 헬퍼
	void LoadStageLevel();

	// 난이도/타입 결정 헬퍼
	EMapGrade GetMapGradeByFloor(int32 Floor) const;
	EMapType GetRandomTypeFromGrade(EMapGrade Grade) const;

};