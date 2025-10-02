// Combat/TurnSchedulerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurnSchedulerComponent.generated.h"

class ABattleManager;
class ACombatPawn;
struct FTurnContext;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UTurnSchedulerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurnSchedulerComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<ABattleManager> BattleManagerRef;

public:
	UFUNCTION(BlueprintCallable, Category = "Turn Scheduler")
	ACombatPawn* DetermineNextTurnCombatant(const TArray<ACombatPawn*>& AllCombatants);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Scheduler|Prediction")
	TArray<ACombatPawn*> GetPredictedTurnOrder(int32 MaxPredictionCount = 5) const;

private:
	// 이 클래스 내부에서만 사용하는 데이터 구조체를 private 영역에 함께 정의합니다.
	struct FSimulatedPawnData
	{
		TWeakObjectPtr<ACombatPawn> Pawn;
		float SimulatedActionValue;
		float MovementSpeed;

		FSimulatedPawnData(ACombatPawn* InPawn);
		float GetTimeToReachThreshold(float Threshold) const;
	};

	struct FTimeToActData
	{
		ACombatPawn* NextCombatant = nullptr;
		float MinTimeToAct = MAX_FLT;
	};

	// --- DetermineNextTurnCombatant 헬퍼 함수 ---
	TArray<ACombatPawn*> FindReadyCombatants(const TArray<ACombatPawn*>& AllCombatants) const;
	void SortCombatantsByPriority(TArray<ACombatPawn*>& Combatants) const;
	FTimeToActData CalculateMinTimeToAct(const TArray<ACombatPawn*>& AllCombatants) const;
	void AdvanceAllActionValues(const TArray<ACombatPawn*>& AllCombatants, float TimeDelta);

	// --- GetPredictedTurnOrder 헬퍼 함수 ---
	void AddTurnStackToPrediction(TArray<ACombatPawn*>& OutPredictedOrder, const TArray<FTurnContext>& TurnStack, int32 MaxPredictionCount) const;
	TArray<FSimulatedPawnData> CreateSimulationData(const TArray<ACombatPawn*>& AllCombatants, const TArray<ACombatPawn*>& InitialPrediction) const;
	void RunTurnPredictionSimulation(TArray<ACombatPawn*>& OutPredictedOrder, TArray<FSimulatedPawnData>& SimPawns, int32 MaxPredictionCount) const;
	float CalculateMinTimeToAct_Simulated(const TArray<FSimulatedPawnData>& SimPawns) const;
	TArray<FSimulatedPawnData*> FindAndSortReadyPawns_Simulated(TArray<FSimulatedPawnData>& SimPawns) const;
};