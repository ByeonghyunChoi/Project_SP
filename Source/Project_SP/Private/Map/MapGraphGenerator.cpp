// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/MapGraphGenerator.h"
#include "Map/MapNode.h"

//UMapNode* UMapGraphGenerator::GenerateStageGraph(UObject* Outer, int32 StageNumber)
//{
//	if (!Outer)
//	{
//		UE_LOG(LogTemp, Log, TEXT("MapGenerator의 소유자가 없습니다."));
//		return nullptr;
//	}
//
//	//맵의 모든 노드를 층별로 저장할 TMap자료구조
//	TMap<int32, TArray<UMapNode*>> AllNodesByLayer;
//	//총 10개의 층
//	AllNodesByLayer.Reserve(10);
//
//	//각 스테이지의 1번째 맵은 일반 전투맵 고정
//	UMapNode* RootNode = NewObject<UMapNode>(Outer);
//	RootNode->InitializeNode(EMapType::NormalBattle, 0);
//	AllNodesByLayer.Add(0, TArray<UMapNode*>{RootNode});
//
//	//이전 맵 노드를 부모로 하여 자식들 맵 생성 시작
//	GenerateChildrenRecursive(Outer, RootNode, 0, AllNodesByLayer);
//
//	//각 스테이지의 8층 준비맵 노드 생성
//	UMapNode* PrepareNode = NewObject<UMapNode>(Outer);
//	PrepareNode->InitializeNode(EMapType::Prepare, 8);
//	AllNodesByLayer.Add(8, TArray<UMapNode*>{PrepareNode});
//
//	//각 스테이지의 9층 보스맵 노드 생성
//	UMapNode* BossNode = NewObject<UMapNode>(Outer);
//	BossNode->InitializeNode(EMapType::BossBattle, 9);
//	AllNodesByLayer.Add(9, TArray<UMapNode*>{BossNode});
//
//	//8층에서 9층 단일 연결
//	PrepareNode->AddChildNode(BossNode);
//
//	//7층에서 8층(준비 맵)으로 강제 합류
//	if (AllNodesByLayer.Contains(7))
//	{
//		for (UMapNode* NodeInLayer7 : AllNodesByLayer[7])
//		{
//			NodeInLayer7->AddChildNode(PrepareNode);
//		}
//	}
//
//	//시작 노드 반환
//	return RootNode;
//}
//
//void UMapGraphGenerator::GenerateChildrenRecursive(UObject* Outer, UMapNode* ParentNode, int32 CurrentLayer, TMap<int32, TArray<UMapNode*>>& AllNodesByLayer)
//{
//	// 7층에 도달하면 재귀 생성 종료
//	if (CurrentLayer >= 7) 
//	{
//		return;
//	}
//
//	int32 NextLayer = CurrentLayer + 1;
//
//	//다음 층에 생성해야 할 맵 타입 목록을 가져옴.
//	TArray<EMapType> TypesForNextLayer = GetMapTypesForNextLayer(NextLayer);
//
//	//해당 타입들로 자식 노드를 생성하고 부모에 연결
//	for (EMapType ChildType : TypesForNextLayer)
//	{
//		UMapNode* NewChildNode = NewObject<UMapNode>(Outer);
//		NewChildNode->InitializeNode(ChildType, NextLayer);
//		ParentNode->AddChildNode(NewChildNode);
//		AllNodesByLayer.FindOrAdd(NextLayer).Add(NewChildNode);
//		GenerateChildrenRecursive(Outer, NewChildNode, NextLayer, AllNodesByLayer);
//	}
//}
//
//TArray<EMapType> UMapGraphGenerator::GetMapTypesForNextLayer(int32 NextLayerIndex)
//{
//	switch (NextLayerIndex)
//	{
//		//3번째, 6번째 맵에서 강적, 광대 맵이 확정적으로 각각 연결(순서만 다르게 설정)
//		case 2:
//		case 5:
//		{
//			if (FMath::RandBool())
//			{
//				return { EMapType::StrongEnemyBattle, EMapType::Jester };
//			}
//			return { EMapType::Jester, EMapType::StrongEnemyBattle };
//		}
//
//		// 일반 맵에서 9:1비율로 일반 전투, 휴식 맵 생성
//		case 1:
//		case 3:
//		case 4:
//		case 6:
//		case 7:
//		{
//			TArray<EMapType> NormalChoices;
//
//			EMapType TypeA = (FMath::FRand() < RestChance) ? EMapType::Rest : EMapType::NormalBattle;
//			NormalChoices.Add(TypeA);
//
//			EMapType TypeB = (FMath::FRand() < RestChance) ? EMapType::Rest : EMapType::NormalBattle;
//			NormalChoices.Add(TypeB);
//
//			return NormalChoices;
//		}
//		//1번째, 9번째, 10번째 맵은 고정 맵이기 떄문에 GetMapTypesForNextLayer함수로 연결되지 않음
//		default:
//			return { EMapType::NormalBattle, EMapType::NormalBattle };
//	}
//}
