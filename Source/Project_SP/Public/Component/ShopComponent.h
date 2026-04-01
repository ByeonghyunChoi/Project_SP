#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ShopDataStructs.h" 
#include "ShopComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShopComponent();

protected:
	// [설정] 이 상인이 판매할 아이템 목록 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop Settings")
	TObjectPtr<class UDataTable> ShopItemTable;

public:
	// 데이터 테이블을 읽어서 아이템 리스트를 반환하는 함수
	UFUNCTION(BlueprintCallable, Category = "Shop")
	TArray<FShopItemRow> GetShopItems() const;
};