#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapInfo.h"
#include "MapBase.generated.h"

class USceneComponent;
class UMapNode;
class APortalActor;
class ARewardBox;

UCLASS()
class PROJECT_SP_API AMapBase : public AActor
{
	GENERATED_BODY()

public:
	AMapBase();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type")
	EMapType CurrentMapType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMapState CurrentMapState;

	// 플레이어 시작 위치를 가진 액터 (Tag: PlayerStartPoint)
	UPROPERTY(VisibleInstanceOnly, Category = "Spawn Settings")
	AActor* LevelPlayerStartActor;

	// 연결된 다음 맵들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Logic")
	TArray<UMapNode*> NextNodeOptions;

	// 배치된 포탈 목록
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Map Logic")
	TArray<APortalActor*> PortalActors;

	// 보상 상자
	/*UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Map Logic")
	TObjectPtr<ARewardBox> RewardBox;*/

	UPROPERTY(EditDefaultsOnly, Category = "Map Logic|Reward")
	TObjectPtr<UDataTable> RewardDataTable;
	//유물 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Map Logic|Reward")
	TObjectPtr<UDataTable> RelicDataTable;

	// 보상 획득 시 호출될 콜백
	UFUNCTION()
	void OnRewardBoxOpened();

	FName GetRewardRowNameByMapType() const;

	//맵에서 재생할 BGM
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> StageBGM;

public:
	// [핵심] 맵 로직 시작 (몬스터/포탈 스폰 -> 부모 호출)
	UFUNCTION(BlueprintNativeEvent, Category = "Map Logic")
	void BeginMapLogic();
	virtual void BeginMapLogic_Implementation();

	// [핵심] 전투 승리 시 호출 (보상 스폰 -> 부모 호출)
	UFUNCTION(BlueprintNativeEvent, Category = "Map Logic")
	void OnCombatFinished(bool bPlayerWon);
	virtual void OnCombatFinished_Implementation(bool bPlayerWon);

	void InitializeNextNodes(const TArray<UMapNode*>& ChildNodes);
	void ClearMapElements();

	// 스폰용 함수 (BP에서 사용하거나 내부 사용)
	/*UFUNCTION(BlueprintCallable, Category = "Map Logic")
	void SpawnRewardBox();*/

protected:
	// 포탈을 열거나 닫는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Logic")
	void ActivatePortals();

	UFUNCTION(BlueprintCallable, Category = "Map Logic")
	void InitPortalsToInactive();

public:
	// Getter / Setter
	void SetMapType(const EMapType& NewMapType);
	EMapType GetMapType() const;
	void SetMapState(const EMapState& NewMapState);
	EMapState GetMapState() const;

	FVector GetPlayerStartLocation() const;
	FRotator GetPlayerStartRotation() const;
};