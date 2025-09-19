// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InventoryComponent.h"
#include "Items/ItemBase.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(FName ItemID, int32 Count)// TArray로 수정해서 받아서 한번에 추가
{
	// 데이터 테이블에서 해당 아이템의 중첩 가능 여부를 확인
	const FString ItemDataTablePath = TEXT("/Game/DataTable/DT_ItemData.DT_ItemData"); // 데이터 테이블 경로
	UDataTable* ItemDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ItemDataTablePath));

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't Load Item Data Table"));
		return;
	}

	// 데이터 테이블에서 해당 아이템의 중첩 가능 여부를 확인
	FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));

	if (FoundData && FoundData->bCanStack)
	{
		// 중첩 가능한 아이템일 경우, 인벤토리에서 이미 존재하는지 확인
		for (UItemBase* ExistingItem : Items)
		{
			if (ExistingItem->ItemID == ItemID)
			{
				// 이미 존재하는 아이템이 있다면, 개수를 증가
				ExistingItem->ItemCount += Count;
				OnInventoryUpdated.Broadcast();
				return;
			}
		}
	}

	// 중첩 불가능하거나, 인벤토리에 없는 아이템일 경우 새로 추가
	UItemBase* NewItem = NewObject<UItemBase>(this);
	NewItem->InitializeItem(ItemID, Count);
	Items.Add(NewItem);

	//UI 업데이트를 위해 델리게이트를 호출
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItem(UItemBase* ItemToRemove)
{
	// 아이템이 유효한지 확인
	if (ItemToRemove)
	{
		// 인벤토리에서 아이템 제거
		Items.Remove(ItemToRemove);

		// 제거된 아이템의 객체 메모리 해제
		ItemToRemove->ConditionalBeginDestroy();

		// UI 업데이트를 위해 델리게이트를 호출
		OnInventoryUpdated.Broadcast();
	}
}