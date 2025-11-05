// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Combat/CombatTypes.h"
#include "MapNode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UMapNode : public UObject
{
	GENERATED_BODY()
	
	//변수 섹션
public:
	//해당 맵 노드의 고유 식별자(클리어 여부 확인 용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Node")
	FGuid NodeID;

	//해당 맵 노드의 종류
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Node")
	EMapType MapType;

	//보상 정보(나중에 구현)

	//해당 맵 노드가 속한 라운드(1-1, 1-2 ...)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Node")
	int32 LayerIndex;

	//해당 맵 노드에서 이동할 수 있는 다음 노드들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Node")
	TArray<TObjectPtr<UMapNode>> ChildNodes;

	//함수 섹션
public:
	// 노드 초기화 함수
	void InitializeNode(EMapType InMapType, int32 InLayerIndex);

	//자식 노드 연결 함수
	void AddChildNode(UMapNode* ChildNode);
};
