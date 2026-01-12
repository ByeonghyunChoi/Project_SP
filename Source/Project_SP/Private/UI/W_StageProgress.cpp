//#include "UI/W_StageProgress.h"
//#include "UI/W_StageNode.h"
//#include "Components/HorizontalBox.h"
//#include "Components/HorizontalBoxSlot.h"
//#include "Components/TextBlock.h"
//
//void UW_StageProgress::InitializeMap(int32 ChapterIndex, int32 StageIndex, const TArray<EMapType>& StageList, int32 CurrentStageIndex)
//{
//    if (!NodeContainer || !NodeWidgetClass) return;
//
//    // 1. 챕터 텍스트 설정 (예: "1-1")
//    if (ChapterText)
//    {
//        ChapterText->SetText(FText::FromString(FString::Printf(TEXT("%d-%d"), ChapterIndex, StageIndex)));
//    }
//
//    // 2. 기존 노드 초기화
//    NodeContainer->ClearChildren();
//    CreatedNodes.Empty();
//
//    // 3. 노드 생성 및 배치
//    for (int32 i = 0; i < StageList.Num(); ++i)
//    {
//        UW_StageNode* NewNode = CreateWidget<UW_StageNode>(this, NodeWidgetClass);
//        if (NewNode)
//        {
//            // 현재 인덱스와 같으면 화살표 켜기
//            bool bIsCurrent = (i == CurrentStageIndex);
//            NewNode->SetupNode(StageList[i], bIsCurrent);
//
//            // 컨테이너에 추가
//            UHorizontalBoxSlot* NewSlot = NodeContainer->AddChildToHorizontalBox(NewNode);
//
//            // 간격 조정 (가운데 정렬, 패딩 등)
//            if (NewSlot)
//            {
//                NewSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
//                NewSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
//                NewSlot->SetPadding(FMargin(12.0f, 0.0f)); // 노드 사이 간격
//            }
//
//            CreatedNodes.Add(NewNode);
//        }
//    }
//}
//
//void UW_StageProgress::UpdatePlayerPosition(int32 NewStageIndex)
//{
//    // 모든 노드를 순회하며 화살표 갱신
//    for (int32 i = 0; i < CreatedNodes.Num(); ++i)
//    {
//        if (CreatedNodes[i])
//        {
//            CreatedNodes[i]->SetIsCurrent(i == NewStageIndex);
//        }
//    }
//
//    ChapterText->SetText(FText::FromString(FString::Printf(TEXT("1-%d"), NewStageIndex + 1)));
//}