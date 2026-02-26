// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SPCombatTurnManager.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECT_SP_API ASPCombatTurnManager : public AInfo
{
	GENERATED_BODY()

public:
	ASPCombatTurnManager();

	// 전투 참가자 등록 (GameMode에서 호출)
	void InitializeParticipants(const TArray<AActor*>& InParticipants);

	// 다음 행동할 유닛 계산 (게이지 100 찰 때까지 시뮬레이션)
	AActor* CalculateNextTurn();

	// 죽은 유닛 제외 (전투 중 사망 시 호출)
	void RemoveParticipant(AActor* DeadActor);

	// 특정 유닛의 게이지 강제 설정 (선제공격 보너스, 턴 종료 시 리셋용)
	void SetActionGauge(AActor* Target, float NewValue);

	// 최대 행동 게이지 (상수: 100)
	UFUNCTION(BlueprintPure, Category = "TurnManager")
	static float GetMaxActionGauge() { return MaxActionGauge; }

public:
	static constexpr float MaxActionGauge = 100.0f;

private:
	// 전체 참가자 목록
	UPROPERTY(VisibleAnywhere, Category = "TurnManager")
	TArray<AActor*> Participants;

	// 행동 가능 상태가 된 유닛들의 대기열
	UPROPERTY(VisibleAnywhere, Category = "TurnManager")
	TArray<AActor*> TurnQueue;

	// GAS Helper Functions
	float GetSpeed(AActor* Target) const;
	float GetActionGauge(AActor* Target) const;
};
