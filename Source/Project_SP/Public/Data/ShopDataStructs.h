#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" //
#include "ShopDataStructs.generated.h" //

/**
 * 
 */
USTRUCT(BlueprintType)
struct FShopItemRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 아이템 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// 판매 가격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 100;

	// 재고 (-1이면 무한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stock = -1;
};