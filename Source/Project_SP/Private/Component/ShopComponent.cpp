// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ShopComponent.h"

UShopComponent::UShopComponent()
{
	
}

TArray<FShopItemRow> UShopComponent::GetShopItems() const
{
	TArray<FShopItemRow> Items;

	// 데이터 테이블이 설정되어 있는지 확인
	if (ShopItemTable)
	{
		// 테이블의 모든 행(Row)을 가져옴.
		FString ContextString;
		TArray<FShopItemRow*> Rows;
		ShopItemTable->GetAllRows<FShopItemRow>(ContextString, Rows);

		// 가져온 행들을 리스트에 담음.
		for (FShopItemRow* Row : Rows)
		{
			if (Row)
			{
				Items.Add(*Row);
			}
		}
	}
	else
	{
		// 테이블이 없을 경우 경고 로그
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: 데이터 테이블이 설정되지 않았습니다!"));
	}

	return Items;
}


