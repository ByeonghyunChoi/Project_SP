// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

class UBoxComponent;
class UWidgetComponent;
class UMapNode;

UCLASS()
class PROJECT_SP_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalActor();

	//Portal Structure Section
protected:
	//캐릭터와 겹치는 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UBoxComponent> OverlapVolume;
	//포탈의 루트 컴포넌트 기준점을 위한 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<USceneComponent> PortalRoot;
	//보상 정보를 보여줄 위젯 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UWidgetComponent> InfoWidget;

	//Map Transfer Logic
protected:
	// 다음 맵의 노드 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal Logic")
	TObjectPtr<UMapNode> TargetNodeData;

	//다음 스테이지로 가는 포탈인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal Logic")
	bool bIsStageExitPortal = false;

public:
	//포탈에 맵 데이터와 UI데이터 갱신 함수
	void InitializePortalData(UMapNode* NodeData);

	//스테이지 이동 포탈 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Portal Logic")
	void ActivateAsStageExitPortal();

protected:
	//블루프린트에서 갱신된 데이터를 기반으로 UI에 아이콘을 표시해주는 함수(BP에서 재사용해서 사용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Portal Logic")
	void UpdatePortalWidget();

	//오버랩 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
