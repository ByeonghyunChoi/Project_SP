#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" //
#include "GameplayEffect.h"
#include "Data/Asset/RelicDefinition.h"
#include "ShopDataStructs.generated.h" //

/**
 * 
 */
 // 재화 종류 정의
UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	Energy   UMETA(DisplayName = "불완전한 기운"),
	Sand     UMETA(DisplayName = "모래"),
	Fragment UMETA(DisplayName = "파편")
};

// 재화 상점 데이터 테이블용 구조체
USTRUCT(BlueprintType)
struct FCurrencyShopRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECurrencyType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 1; // 판매 수량 (예: 모래 100개)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 100; // 가격 (골드)
};

 // 유물 풀 전용 구조체 (기존 상점 구조체를 건드리지 않음!)
USTRUCT(BlueprintType)
struct FRelicPoolRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 우리가 이미 만들어둔 유물 데이터 에셋만 꽂으면 끝!
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class URelicDefinition> RelicAsset;

	// 상점용 가격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 100;
};

USTRUCT(BlueprintType)
struct FShopItemRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 아이템 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// 3. UI 표시용 기본 정보 (회복약 같은 일반 아이템용)
	// 유물이 없을 때는 이 정보를 사용하고, 유물이 있다면 에셋의 정보를 우선시합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	// 4. 상점용 데이터 (에셋에는 없는, 상점에서만 필요한 정보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Price = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Stock = 1;

	// 5. 효과 관련 (회복약 등 일반 아이템용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float ValueAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	// 🌟 유물 에셋을 직접 담을 변수 추가!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TObjectPtr<const URelicDefinition> RelicDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Amount = 0; // 🌟 판매할 재화의 수량을 담는 칸 추가!

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	ECurrencyType CurrencyType; // 🌟 어떤 재화인지 구분하는 타입 추가!
};