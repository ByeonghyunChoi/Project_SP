//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "EquipmentSystemComponent.generated.h"
//
//class UOpartsBase;
//class UCrystalSkullOparts; // 오파츠 자식 클래스 전방 선언
//class UJadeClockOparts;
//class UGoldBugOparts;
//
//UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
//class PROJECT_SP_API UEquipmentSystemComponent : public UActorComponent
//{
//	GENERATED_BODY()
//
//public:
//	UEquipmentSystemComponent();
//
//protected:
//	virtual void BeginPlay() override;
//
//	// [1] APlayerCharacter에서 미리 장착된 3개의 오파츠 포인터를 받을 변수
//	// 이 변수들은 APlayerCharacter::BeginPlay()에서 할당되어야 합니다.
//	UPROPERTY()
//	TObjectPtr<UCrystalSkullOparts> CrystalSkull;
//
//	UPROPERTY()
//	TObjectPtr<UJadeClockOparts> JadeClock;
//
//	UPROPERTY()
//	TObjectPtr<UGoldBugOparts> GoldBug;
//
//	/*UPROPERTY()
//	TObjectPtr<UAttributesComponent> AttributesComp;*/
//
//public:	
//	// [2] APlayerCharacter에서 초기화 시 포인터를 설정하는 함수
//	void InitializeOpartsPointers(UCrystalSkullOparts* Skull, UJadeClockOparts* Clock, UGoldBugOparts* Bug);
//
//	// [3] UI에서 호출될 개별 활성화/비활성화 함수 (핵심)
//	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
//	void ActivateCrystalSkull();
//
//	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
//	void ActivateJadeClock();
//
//	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
//	void ActivateGoldBug();
//
//	// UI구성시 마지막으로 설정한 아티팩트 불러오기
//	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
//	void ActivateLastOparts();
//
//	// 활성화 시킨 오파츠의 번호 저장
//	UPROPERTY(BlueprintReadOnly, Category = "Oparts Control")
//	int32 ActiveOpartsIndex = 0;
//
//	// 오파츠를 교체하고 스탯을 갱신하는 함수
//	UFUNCTION(BlueprintCallable, Category = "Oparts")
//	void SetActiveOparts(int32 NewIndex);
//
//	//현재 활성화된 오파츠 객체를 반환하는 함수
//	UFUNCTION(BlueprintPure, Category = "Oparts Control")
//	UOpartsBase* GetActiveOparts() const;
//
//	// [신규] 현재 장착 중인 오파츠 강화 시도
//	UFUNCTION(BlueprintCallable, Category = "Oparts")
//	void TryUpgradeOparts();
//
//private:
//	// 특정 컴포넌트의 활성화 상태를 변경하는 헬퍼 함수
//	void SetOpartsActiveState(UOpartsBase* Oparts, bool bActive);
//};
