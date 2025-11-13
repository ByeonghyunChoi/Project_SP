// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CombatTypes.h"
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
	// Sets default values for this actor's properties
	AMapBase();

	//Map Structure Section
protected:
	//맵 종류
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type")
	EMapType CurrentMapType;
	//맵 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMapState CurrentMapState;
	//플레이어 시작 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	TObjectPtr<USceneComponent> PlayerStartPoint;

	//Map Transfer, Portal Section
protected:
	//연결된 맵
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Logic")
	TArray<UMapNode*> NextNodeOptions;
	//배치된 포탈
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Map Logic")
	TArray<APortalActor*> PortalActors;
	//보상 상자
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Map Logic")
	TObjectPtr<ARewardBox> RewardBox;

	//Manager Call Section
public:
	//맵에 배치된 오브젝트들 활성화 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Map Logic")
	void BeginMapLogic();
	virtual void BeginMapLogic_Implementation();

	//전투에 승리하면 작동하는 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Map Logic")
	void OnCombatFinished(bool bPlayerWon);
	virtual void OnCombatFinished_Implementation(bool bPlayerWon);

	//다음 맵 목록을 넘겨주는 함수
	void InitializeNextNodes(const TArray<UMapNode*>& ChildNodes);

	//맵이 가지고 있는 모든 액터를 파괴하는 함수
	void ClearMapElements();

	//Common Function Section
protected:
	//포탈 활성화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Map Logic")
	void ActivatePortals();

	//Getter, Setter Section
public:
	void SetMapType(const EMapType& NewMapType);
	EMapType GetMapType() const;

	void SetMapState(const EMapState& NewMapState);
	EMapState GetMapState() const;

	FVector GetPlayerStartLocation() const;
	FRotator GetPlayerStartRotation() const;

};
