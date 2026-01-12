// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/W_StageNode.h"
#include "Components/Image.h"

//void UW_StageNode::SetupNode(EMapType NodeType, bool bIsCurrent)
//{
//    // 1. 타입에 따른 색상 변경
//    FLinearColor SelectedColor = NormalColor;
//    switch (NodeType)
//    {
//        // [그룹 1] 일반 맵 (일반 전투, 휴식) -> 파란색/흰색 등 설정한 NormalColor
//    case EMapType::NormalBattle:
//    case EMapType::Rest:
//        SelectedColor = NormalColor;
//        break;
//
//        // [그룹 2] 에픽 맵 (강적, 광대) -> 보라색 (StrongEnemyBattleColor)
//    case EMapType::StrongEnemyBattle:
//    case EMapType::Jester:
//        SelectedColor = EpicColor; // 혹은 EpicColor 변수 사용
//        break;
//
//        // [그룹 3] 준비 맵 -> 노란색/흰색 (PrepareColor)
//    case EMapType::Prepare:
//        SelectedColor = PrepareColor;
//        break;
//
//        // [그룹 4] 보스 맵 -> 빨간색 (BossColor)
//    case EMapType::BossBattle:
//        SelectedColor = BossColor;
//        break;
//    }
//
//    if (NodeImage)
//    {
//        NodeImage->SetBrushTintColor(SelectedColor);
//    }
//
//    // 2. 현재 위치 표시 (화살표)
//    SetIsCurrent(bIsCurrent);
//}

//void UW_StageNode::SetIsCurrent(bool bIsCurrent)
//{
//    if (ArrowImage)
//    {
//        // 현재 위치면 화살표 보이기, 아니면 숨기기
//        ArrowImage->SetVisibility(bIsCurrent ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
//    }
//}

