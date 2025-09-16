// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/ItemBase.h"
#include "Engine/DataTable.h"

void UItemBase::InitializeItem(FName InItemID, int32 InCount)
{
	// ItemID와 ItemCount 설정
	ItemID = InItemID;
	ItemCount = InCount;

	static ConstructorHelpers::FObjectFinder<UDataTable> ItemDataTableFinder(TEXT("/Game/DataTable/DT_ItemData.DT_ItemData"));

	if (ItemDataTableFinder.Succeeded())
	{
		UDataTable* ItemDataTable = ItemDataTableFinder.Object;

		// 아이템 ID(Row Name)를 기븐으로 데이터 테이블에서 해당 행을 찾는다.
		FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));

		if (FoundData)
		{
			// 찾은 데이터를 ItemData에 복사
			ItemData = *FoundData;
		}
	}
}

