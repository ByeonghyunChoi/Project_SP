#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapInfo.h"       // EMapType, EMapGrade 등 정의
#include "Engine/DataTable.h"
#include "Data/CombatEncounterData.h"
#include "Data/SPDataStructs.h"
#include "Data/RewardDataStructs.h"
#include "SubSystem/SPCombatSubsystem.h" // ECombatAdvantage 정의
#include "Component/InventoryComponent.h"
#include "MapManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMapLocationChanged, int32, NewStage, int32, NewFloor);

// 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FMapLevelData : public FTableRowBase
{
	GENERATED_BODY()

	// 1~3층(일반, 준비)에서 공통으로 사용할 물리적 레벨 (.umap)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> NormalLevelReference;

	// 4층(보스)에서만 사용할 물리적 레벨 (.umap)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> BossLevelReference;

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

	// 맵 바뀐거 알림용
	UPROPERTY(BlueprintAssignable, Category = "Map")
	FOnMapLocationChanged OnMapLocationChanged;

	bool IsInBattleMap() { return bIsBattleActive; }
	
	//로비 맵 판독용
	bool GetIsInLobby() const { return bIsInLobby; }

	// 일반 전투 몬스터 생성 함수
	void PreGenerateAllNormalEncounters();

	// 전투 종료 시 확정 보상 계산
	UFUNCTION(BlueprintCallable, Category = "Reward")
	FRewardResult CalculateCombatRewards(const TArray<EMonsterRank>& DefeatedRanks, int32 Stage, EMapType MapType);

	// 상호작용 (상자/회복) 시 가중치 랜덤 보상 계산
	UFUNCTION(BlueprintCallable, Category = "Reward")
	FRewardResult GenerateInteractableReward(bool bIsHealingObject, int32 Stage, EMapType MapType);

	//Getter
	UFUNCTION(BlueprintPure, Category = "MapProgress")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	int32 GetCurrentFloor() const { return CurrentFloor; }

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	EMapType GetCurrentMapType() const { return CurrentMapType; }

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	EMapState GetCurrentRoomState() const { return CurrentRoomState; }

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	TArray<EMapType> GetCurrentPortalOptions() const { return CurrentPortalOptions; }

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	int32 GetMaxFloors() const { return 5; }

	TMap<int32, FSavedEncounterData> GetPreGeneratedEncounters() const { return PreGeneratedEncounters; }

	// 레벨 계산기
	UFUNCTION(BlueprintPure, Category = "MapProgress")
	int32 CalculateMonsterLevel() const;

	// 약점 생성기
	UFUNCTION(BlueprintPure, Category = "MapProgress")
	FGameplayTagContainer GenerateRandomWeaknesses(int32 Count);

	// 인카운터 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	UCombatEncounterData* GenerateFieldEncounter();

	//일반 몬스터 스폰 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FTransform GetFieldSpawnTransform(int32 Index);

	UFUNCTION(BlueprintPure, Category = "MapProgress")
	EMapGrade GetMapGradeForUI(int32 Floor) const;

	//Setter
	void SetCurrentRoomState(EMapState NewState) { CurrentRoomState = NewState; }

	// 로드 시스템
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ResumeRunFromSave(int32 SavedStage, int32 SavedFloor, EMapType SavedMapType, EMapState SavedRoomState, FTransform SavedTransform, bool bSavedInLobby, TArray<EMapType> SavedPortalOptions);

	// 필드 복귀 시 화면에 띄워줄 보상 대기열 (재화 종류, 수량)
	UPROPERTY(BlueprintReadWrite, Category = "Reward")
	TMap<EResourceType, int32> PendingToastRewards;

protected:
	// 로비 레벨 레퍼런스 (에디터에서 경로 확인 필요)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LobbyLevelReference;

	// 맵 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	UPROPERTY()
	TObjectPtr<class USPStageMonsterPoolData> StagePoolDataAsset;

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

	// 몬스터 명부
	UPROPERTY()
	TMap<int32, FSavedEncounterData> PreGeneratedEncounters;

private:
	// 맵 생성 및 플레이어 이동 처리
	void SpawnMapActor(EMapType MapType);

	// 레벨 로드 완료 시 호출되는 콜백 (위치 복구 핵심)
	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

	// 스테이지 레벨 로드 헬퍼
	void LoadStageLevel();

	// 가중치 기반으로 맵을 정하는 함수
	EMapType PickAndRemoveWeightedMap(TMap<EMapType, int32>& InOutWeightPool);

};