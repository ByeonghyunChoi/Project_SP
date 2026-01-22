#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapInfo.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "MapManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FMapLevelData : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> LevelReference;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> BackgroundMusic;
};

UCLASS()
class PROJECT_SP_API UMapManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void StartNewRun();
	void MoveToNextFloor(EMapType SelectedType);
	TArray<EMapType> GenerateNextFloorOptions();
	void EnterBattle(TSoftObjectPtr<UWorld> BattleLevelRes);
	void ExitBattle();
	void InitializeCurrentMap(class AMapBase* InMapActor);

protected:
	EMapGrade GetMapGradeByFloor(int32 Floor) const;
	EMapType GetRandomTypeFromGrade(EMapGrade Grade) const;
	void LoadStageLevel();

	// 비동기 레벨 로딩 완료 콜백
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
};