// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/ArtifactItem.h"
#include "Engine/DataTable.h"
#include "Data/ArtifactData.h"

UArtifactItem::UArtifactItem()
{
}

void UArtifactItem::InitializeArtifact(FName InItemID, int32 InCount)
{
	// 부모클래스의 InitialLizeItem 함수를 호출하여 기본 아이템 데이터를 설정
    InitializeItem(InItemID, InCount);

    // StaticLoadObject를 사용하여 데이터 테이블을 로드합니다.
    const FString ArtifactDataTablePath = TEXT("/Game/DataTable/DT_ArtifactData.DT_ArtifactData");
    UDataTable* ArtifactDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ArtifactDataTablePath));

    if (!ArtifactDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("아티팩트 데이터 테이블을 로드할 수 없습니다!"));
        return;
    }

    // 아이템 ID를 기반으로 아티팩트 데이터를 찾습니다.
    FArtifactData* FoundArtifactData = ArtifactDataTable->FindRow<FArtifactData>(InItemID, TEXT(""));

    if (FoundArtifactData)
    {
        // 찾은 데이터를 ArtifactData 변수에 복사합니다.
        ArtifactData = *FoundArtifactData;
    }
}