// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ShopComponent.h"
#include "Engine/DataTable.h"

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShopComponent::BeginPlay()
{
	Super::BeginPlay();

	// 🌟 레벨이 시작될 때 딱 한 번! 데이터 테이블 원본을 내 주머니(RuntimeShopItems)로 복사해옵니다.
	if (ShopDataTable)
	{
		TArray<FShopItemRow*> AllRows;
		ShopDataTable->GetAllRows<FShopItemRow>(TEXT("ShopComponent Context"), AllRows);
		for (FShopItemRow* Row : AllRows)
		{
			if (Row)
			{
				RuntimeShopItems.Add(*Row);
			}
		}
	}
}

void UShopComponent::ConsumeStock(FName ItemID)
{
	// 🌟 주머니를 뒤져서 방금 산 아이템의 재고를 1 깎습니다.
	for (FShopItemRow& Item : RuntimeShopItems)
	{
		if (Item.ItemID == ItemID)
		{
			if (Item.Stock > 0)
			{
				Item.Stock--;
				UE_LOG(LogTemp, Warning, TEXT("[%s] 재고 감소! 남은 수량: %d"), *ItemID.ToString(), Item.Stock);
			}
			return; // 찾았으니 종료
		}
	}
}