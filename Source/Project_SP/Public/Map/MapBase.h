#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapInfo.h"
#include "MapBase.generated.h"

UCLASS()
class PROJECT_SP_API AMapBase : public AActor
{
	GENERATED_BODY()

public:
	AMapBase();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | Layout")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, Category = "Map | Setup")
	TSubclassOf<class APortalActor> PortalClass;

	// 스폰할 보상 상자 클래스
	UPROPERTY(EditAnywhere, Category = "Map | Setup")
	TSubclassOf<class ARewardBox> RewardChestClass;

	// 관리 중인 스폰된 포탈들
	UPROPERTY()
	TArray<class APortalActor*> SpawnedPortals;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | State")
	EMapState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | State")
	EMapType MapType;

public:
	TArray<FTransform> GetSpawnTransformsByTag(FName PointTag) const;

	UFUNCTION(BlueprintCallable)
	void SetMapState(EMapState NewState);
	void InitializeMap(EMapType InType);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Map | State")
	void OnMapStateChanged(EMapState OldState, EMapState NewState);

private:
	void HandleStateInProgress();
	void HandleStateReward();
	void HandleStateCleared(); 
};