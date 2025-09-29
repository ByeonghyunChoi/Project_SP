// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ArtifactItem.h"
#include "Engine/DataTable.h"
#include "Data/ArtifactData.h"

UArtifactItem::UArtifactItem()
{
}

void UArtifactItem::InitializeArtifactData(const FArtifactData& InData)
{
    ArtifactData = InData;

    // 부모 클래스의 ItemID를 조합하여 고유한 ID를 생성합니다.
    FString ItemIDString =
        UEnum::GetValueAsString(InData.ArtifactGrade) + "_" +
        UEnum::GetValueAsString(InData.ArtifactType) + "_" +
        UEnum::GetValueAsString(InData.ArtifactKinds);

    // 특수 옵션이 있는 경우 ID에 추가 (선택 사항, 데이터 테이블에서 찾기 위해)
    if (InData.SpecialOptionType != ESpecialOptionType::ESOT_None)
    {
        ItemIDString += "_" + UEnum::GetValueAsString(InData.SpecialOptionType);
    }

    FName FinalItemID = FName(*ItemIDString);

    // ItemBase의 기본 데이터를 설정합니다. (ItemID와 ItemCount=1)
    // 이 FinalItemID를 사용하여 DT_ItemData에서 아티팩트의 이름, 아이콘 등을 로드합니다.
    InitializeItem(FinalItemID, 1);
}