// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/CombatEncounterData.h"
#include "SPCombatSubsystem.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ECombatAdvantage : uint8
{
	PlayerAdvantage, //플레이어 선공
	EnemyAdvantage  //플레이어 피격
};

UCLASS()
class PROJECT_SP_API USPCombatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 전투 정보를 저장하고 준비시킴
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetPendingEncounter(const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage);
	
	// 저장된 전투 데이터 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	const UCombatEncounterData* GetPendingEncounter() const { return CurrentEncounter; }

	// 저장된 어드밴티지 상태 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ECombatAdvantage GetAdvantageState() const { return CurrentAdvantage; }

private:
	// 다음 전투에 사용할 몬스터 데이터 (안전하게 보관 중)
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<const UCombatEncounterData> CurrentEncounter;

	// 선공 여부
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	ECombatAdvantage CurrentAdvantage;
};
