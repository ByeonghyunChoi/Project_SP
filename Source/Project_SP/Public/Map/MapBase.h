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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | Layout")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, Category = "Map | Setup")
	TSubclassOf<class APortalActor> PortalClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | State")
	EMapState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map | State")
	EMapType MapType;

public:
	TArray<FTransform> GetSpawnTransformsByTag(FName PointTag) const;
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