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

	// 유물 풀 데이터 테이블 (새로 만든 FRelicPoolRow 구조체를 사용하는 테이블)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	class UDataTable* RelicPoolDataTable;

	//  게임 중에 깎이는 진짜 재고 주머니 (실시간)
	UPROPERTY(BlueprintReadWrite, Category = "Shop")
	TArray<FShopItemRow> RuntimeShopItems;

	//  재고 차감 함수
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ConsumeStock(FName ItemID);

	//  랜덤 유물 생성 함수 (진열대에 유물 하나 만들어주는 역할)
	UFUNCTION(BlueprintCallable, Category = "Shop")
	FShopItemRow GenerateRandomRelic();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ApplyRelicToPlayer(URelicDefinition* RelicAsset, AActor* TargetActor);


	UPROPERTY(EditAnywhere, Category = "Shop")
	UDataTable* CurrencyDataTable; // 에디터에서 재화 테이블 연결용

	UFUNCTION(BlueprintCallable, Category = "Shop")
	FShopItemRow GenerateRandomCurrency();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void GiveCurrencyToPlayer(ECurrencyType Type, int32 Amount, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ResetShopForNextLevel();
};