// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPGASBattleManager.generated.h"

UCLASS()
class PROJECT_SP_API ASPGASBattleManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPGASBattleManager();

	void StartBattle(const TArray<AActor*>& Players, const TArray<AActor*>& Monsters);
	void RequestInterruption(AActor* Interrupter); // 턴 삽입
	void NotifyTurnEnd(); // 유닛이 행동을 마쳤음을 매니저에게 알림

protected:
	//전투 상태 설정
	void SetBattleState(EBattleState NewState);

	//상태 변화에 따라 유닛/시스템에 태그 전달
	void UpdateInterfaceTags(EBattleState NewState);

	//턴 순서 로직
	void RefreshTimeline();
	void AdvanceTimeToNextTurn();

	//참가자의 실제 속도 가져오는 로직
	float GetSpeedFromActor(AActor* Actor) const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	EBattleState InternalState = EBattleState::Ready;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	TArray<FBattleUnit> Timeline;

	UPROPERTY()
	TObjectPtr<AActor> CurrentActiveUnit;
};
