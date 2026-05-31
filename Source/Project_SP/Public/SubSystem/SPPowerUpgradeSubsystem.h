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

	// 게임 시작 시, 저장된 권능 레벨을 싹 읽어와서 캐릭터의 어트리뷰트에 일괄 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void ApplySavedStatUpgradesToPlayer(class APawn* PlayerPawn);

	// UI를 처음 열었을 때(또는 업데이트 시) 화면에 띄워줄 정보를 한 번에 반환하는 함수
	UFUNCTION(BlueprintPure, Category = "PowerUpgrade|Stats")
	void GetStatUpgradeInfo(EPowerUpgradeType StatType, int32& OutCurrentLevel, int32& OutNextCost, float& OutCurrentEffectValue) const;

	// 버튼 7개에 각각 연결할 7개의 업그레이드 함수들
	// 반환값: 성공 여부(bSuccess), 그리고 UI를 즉시 갱신할 수 있는 3가지 정보
	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_ATK(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_DEF(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_Speed(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_HP(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_DamageInc(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_CritChance(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUpgrade|Stats")
	void UpgradeStat_CritDamage(class APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue);

private:
	// 데이터 테이블에서 정보 추출하는 내부 헬퍼 함수
	FPowerUpgradeData* GetUpgradeData(EPowerUpgradeType UpgradeType) const;

	// 7개의 버튼이 공통으로 사용할 내부 실제 업그레이드 로직
	bool TryUpgradeStatInternal(EPowerUpgradeType StatType, class APlayerController* PC);
};
