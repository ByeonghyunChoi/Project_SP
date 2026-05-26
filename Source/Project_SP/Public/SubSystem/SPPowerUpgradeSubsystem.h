// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SPDataStructs.h"
#include "SPPowerUpgradeSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPPowerUpgradeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 서브시스템 탄생 시 호출되는 초기화 함수 오버라이드
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// 에디터(블루프린트)에서 할당해 줄 권능 데이터 테이블
	UPROPERTY(BlueprintReadWrite, Category = "PowerUpgrade")
	class UDataTable* PowerUpgradeDataTable;

	// 1. 특정 권능의 현재 레벨 가져오기
	UFUNCTION(BlueprintPure, Category = "PowerUpgrade")
	int32 GetPowerLevel(EPowerUpgradeType UpgradeType) const;

	// 2. 다음 레벨업에 필요한 비용(파편) 가져오기
	UFUNCTION(BlueprintPure, Category = "PowerUpgrade")
	int32 GetNextLevelCost(EPowerUpgradeType UpgradeType) const;

	// 3. 권능 업그레이드 시도 (파편 차감 후 세이브)
	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade")
	bool TryUpgradePower(EPowerUpgradeType UpgradeType, class APlayerController* PlayerController);

	//특정 권능의 현재 레벨에 따른 '실제 적용 수치(Effect Value)' 가져오기
	UFUNCTION(BlueprintPure, Category = "PowerUpgrade")
	float GetPowerEffectValue(EPowerUpgradeType UpgradeType) const;

	// 새 게임(런)이 시작될 때 적용해야 할 그룹A 보너스 일괄 적용
	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade")
	void ApplyNewRunBonuses();

private:
	// 데이터 테이블에서 정보 추출하는 내부 헬퍼 함수
	FPowerUpgradeData* GetUpgradeData(EPowerUpgradeType UpgradeType) const;
};
