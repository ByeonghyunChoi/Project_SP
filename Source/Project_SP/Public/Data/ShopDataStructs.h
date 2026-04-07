#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" //
#include "GameplayEffect.h"
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

	// [추가] UI에 표시할 이름 (예: "회복 물약")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// [추가] UI에 표시할 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	// 판매 가격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 100;

	// 재고 (-1이면 무한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stock = -1;

	// 적용할 수치 (회복약이면 회복량, 예를 들어 400.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float ValueAmount = 0.0f;

	// 적용할 이펙트 클래스 (회복약일 경우 회복 GE 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<class UGameplayEffect> EffectClass;
};