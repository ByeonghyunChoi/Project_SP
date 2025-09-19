// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemBase; // 전방 선언

//인벤토리 업데이트를 위한 델리게이트 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	// 아이템을 인벤토리에 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID, int32 Count = 1);

	// 인벤토리에서 아이템을 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(UItemBase* ItemToRemove);

	// 인벤토리에 담긴 아이템 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UItemBase*> Items;

	// 인벤토리가 업데이트 될 때마다 호출되는 델리게이트 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
};
