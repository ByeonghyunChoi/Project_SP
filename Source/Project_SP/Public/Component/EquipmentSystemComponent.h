// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentSystemComponent.generated.h"

class UOpartsBase;
class UCrystalSkullOparts; // 오파츠 자식 클래스 전방 선언
class UJadeClockOparts;
class UGoldBugOparts;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UEquipmentSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentSystemComponent();

protected:
	// [1] APlayerCharacter에서 미리 장착된 3개의 오파츠 포인터를 받을 변수
	// 이 변수들은 APlayerCharacter::BeginPlay()에서 할당되어야 합니다.
	UPROPERTY()
	TObjectPtr<UCrystalSkullOparts> CrystalSkull;

	UPROPERTY()
	TObjectPtr<UJadeClockOparts> JadeClock;

	UPROPERTY()
	TObjectPtr<UGoldBugOparts> GoldBug;

public:	
	// [2] APlayerCharacter에서 초기화 시 포인터를 설정하는 함수
	void InitializeOpartsPointers(UCrystalSkullOparts* Skull, UJadeClockOparts* Clock, UGoldBugOparts* Bug);

	// [3] UI에서 호출될 개별 활성화/비활성화 함수 (핵심)
	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
	void ActivateCrystalSkull();

	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
	void ActivateJadeClock();

	UFUNCTION(BlueprintCallable, Category = "Oparts Control")
	void ActivateGoldBug();
private:
	// 특정 컴포넌트의 활성화 상태를 변경하는 헬퍼 함수
	void SetOpartsActiveState(UOpartsBase* Oparts, bool bActive);
};
