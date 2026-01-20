//// 상인 상점 UI만(아이템 사고 팔 수 있게), 시간의 파편 UI, 내 정보 창에서 정보 보이기(모래, 불완전한 기운, 보유 아이템, 스탯 - 그냥 보이는거는 현재 체력(Bar 안에 수치), 현재 보유 골드, 유물(정보창, 일반 둘다))
//// 스탯 적용 구조 개선 - Base = 영구적인거 - current 일시적인거 - 전투에서는 current = 수정이 필요해 보임
//// 시간 남으면 유물을 계속 추가
//#pragma once
//
//#include "CoreMinimal.h"
//#include "UObject/NoExportTypes.h"
//#include "Data/RelicData.h"
//#include "RelicBase.generated.h"
//
//UCLASS(BlueprintType, Blueprintable)
//class PROJECT_SP_API URelicBase : public UObject
//{
//	GENERATED_BODY()
//	
//protected:
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Relic Info")
//	FRelicData RelicInfo;
//
//public:
//	void InitializeRelic(const FRelicData& InData)
//	{
//		RelicInfo = InData;
//	}
//
//	UFUNCTION(BlueprintPure, Category = "Relic Info")
//	FRelicData GetRelicData() const
//	{
//		return RelicInfo;
//	}
//
//	UFUNCTION(BlueprintCallable, Category = "Relic")
//	virtual void OnEquip(AActor* Instigator);
//
//	UFUNCTION(BlueprintCallable, Category = "Relic")
//	virtual void OnUnequip(AActor* Instigator);
//
//	//[신규] 유물의 능력치를 반환하는 함수
//	UFUNCTION(BlueprintCallable,  Category = "Relic")
//	virtual FStatModifiers GetRelicModifiers() const;
//
//protected:
//	// 유물 효과 함수 (파생 클래스에서 구현)
//	virtual void ApplyRelicEffect(AActor* Target);
//
//	virtual void RemoveRelicEffect(AActor* Target);
//
//
//};
