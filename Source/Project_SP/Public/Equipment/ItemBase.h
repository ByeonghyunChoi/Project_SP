// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Data/ItemData.h"
#include "ItemBase.generated.h"

UCLASS(Blueprintable, BlueprintType, DefaultToInstanced)
class PROJECT_SP_API UItemBase : public UObject
{
    GENERATED_BODY()

public:
    // 아이템을 초기화하는 함수. 데이터 테이블의 RowName을 받아 정보를 불러옴.
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItem(FName InItemID, int32 InCount);

	// 아이템의 고유 식별자
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	// 아이템의 데이터 테이블 정보를 가지고 있음
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FItemData ItemData;

	// 아이템의 현재 개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 ItemCount;
};
