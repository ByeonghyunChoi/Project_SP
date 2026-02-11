// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Tag/SPGameplayTags.h"
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

	//전투원 등록
	void InitializeParticipants(const TArray<AActor*>& InParticipants);

	//턴 계산 후 반환
	AActor* CalculateNextTurn();

	//죽은 전투 참가자 제외 함수
	void RemoveParticipant(AActor* DeadActor);

	UFUNCTION(BlueprintPure, Category = "TurnManager")
	static float GetMaxActionGauge() { return MaxActionGauge; }

public:
	// 런타임 상수 행동 게이지 최대 값
	static constexpr float MaxActionGauge = 100.0f;
	
private:
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> Participants;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> TurnQueue;

	float GetSpeed(AActor* Target) const;
	float GetActionGauge(AActor* Target) const;
	void SetActionGauge(AActor* Target, float NewValue);
};
