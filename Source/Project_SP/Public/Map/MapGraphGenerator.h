//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "UObject/NoExportTypes.h"
//#include "MapGraphGenerator.generated.h"
//
//class UMapNode;
///**
// * 
// */
//UCLASS()
//class PROJECT_SP_API UMapGraphGenerator : public UObject
//{
//	GENERATED_BODY()
//	
//public:
//	//맵 그래프 생성 함수
//	UMapNode* GenerateStageGraph(UObject* Outer, int32 StageNumber);
//
//private:
//	//부모 노드에 자식 노드들을 재귀적으로 생성하여 2분기로 연결하는 함수
//	void GenerateChildrenRecursive(UObject* Outer, UMapNode* ParentNode, int32 CurrentLayer,
//		TMap<int32, TArray<UMapNode*>>& AllNodesByLayer);
//	//다음 층에 생성되어야 할 2개의 맵 타입 목록을 반환하는 함수
//	//TArray<EMapType> GetMapTypesForNextLayer(int32 NextLayerIndex);
//
//	//쉼터 맵 생성 확률
//	UPROPERTY(EditAnywhere, Category = "Map Generation|Probability")
//	float RestChance = 0.1f;
//};
