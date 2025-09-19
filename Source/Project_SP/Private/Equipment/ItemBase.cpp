// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/ItemBase.h"
#include "Engine/DataTable.h"
#include "Data/ItemData.h"

void UItemBase::InitializeItem(FName InItemID, int32 InCount)
{
	// ItemID와 ItemCount 설정
	ItemID = InItemID;
	ItemCount = InCount;

	const FString ItemDataTablePath = TEXT("/Game/DataTable/DT_ItemData.DT_ItemData");
	UDataTable* ItemDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ItemDataTablePath));

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("아이템 데이터 테이블을 로드할 수 없습니다!"));
		return;
	}

	// 아이템 ID(Row Name)를 기븐으로 데이터 테이블에서 해당 행을 찾는다.
	FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));

	if (FoundData)
	{
		// 찾은 데이터를 ItemData에 복사
		ItemData = *FoundData;
	}
}

