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
	virtual void BeginPlay() override;

public:
	//  에디터에서 설정할 원본 데이터 테이블 (설계도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	class UDataTable* ShopDataTable;

	//  게임 중에 깎이는 진짜 재고 주머니 (실시간)
	UPROPERTY(BlueprintReadWrite, Category = "Shop")
	TArray<FShopItemRow> RuntimeShopItems;

	//  재고 차감 함수
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ConsumeStock(FName ItemID);
};