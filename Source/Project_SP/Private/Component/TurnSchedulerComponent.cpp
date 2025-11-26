// Combat/TurnSchedulerComponent.cpp

#include "Component/TurnSchedulerComponent.h"
#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"

// FSimulatedPawnData 생성자 및 멤버 함수 구현
UTurnSchedulerComponent::FSimulatedPawnData::FSimulatedPawnData(ACombatPawn* InPawn)
{
	Pawn = InPawn;
	if (InPawn)
	{
		SimulatedActionValue = InPawn->GetBattleTurnComponent()->GetActionValue();
		MovementSpeed = InPawn->GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
	}
	else
	{
		SimulatedActionValue = 0.f;
		MovementSpeed = 0.f;
	}
}

float UTurnSchedulerComponent::FSimulatedPawnData::GetTimeToReachThreshold(float Threshold) const
{
	if (MovementSpeed <= 0.f)
	{
		return MAX_FLT;
	}
	return (Threshold - SimulatedActionValue) / MovementSpeed;
}

// UTurnSchedulerComponent 생성자 및 멤버 함수 구현
UTurnSchedulerComponent::UTurnSchedulerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTurnSchedulerComponent::BeginPlay()
{
	Super::BeginPlay();
	BattleManagerRef = Cast<ABattleManager>(GetOwner());
	if (!BattleManagerRef)
	{
		UE_LOG(LogTemp, Error, TEXT("TurnSchedulerComponent는 ABattleManager 액터에만 부착해야 합니다!"));
	}
}

ACombatPawn* UTurnSchedulerComponent::DetermineNextTurnCombatant(const TArray<ACombatPawn*>& AllCombatants)
{

	// 1. 이미 행동 게이지가 꽉 찬(10000 이상) 캐릭터가 있는지 확인 (우선권 처리)
	TArray<ACombatPawn*> ReadyCombatants = FindReadyCombatants(AllCombatants);
	if (ReadyCombatants.Num() > 0)
	{
		SortCombatantsByPriority(ReadyCombatants);
		return ReadyCombatants[0];
	}

	// 2. 아무도 턴을 못 잡았다면 -> "누가 제일 빨리 도착할까?" 계산 (레이싱)
	float MinTimeNeeded = MAX_FLT;
	ACombatPawn* NextPawn = nullptr;

	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (!Combatant || Combatant->GetCombatPawnState() == ECombatPawnState::Defeated) continue;

		UBattleTurnComponent* TurnComp = Combatant->GetBattleTurnComponent();
		UAttributesComponent* AttrComp = Combatant->GetAttributesComponent();

		if (TurnComp && AttrComp)
		{
			float Speed = AttrComp->GetCurrentStats().fMovementSpeed;
			if (Speed <= 0.f) continue;

			float CurrentVal = TurnComp->GetActionValue();
			float GoalVal = TurnComp->GetActionThreshold(); // 보통 10000

			// 공식: (목표 - 현재) / 속도 = 걸리는 시간
			float TimeNeeded = (GoalVal - CurrentVal) / Speed;

			// 가장 시간이 적게 걸리는(가장 빨리 도착할) 캐릭터 찾기
			if (TimeNeeded < MinTimeNeeded)
			{
				MinTimeNeeded = TimeNeeded;
				NextPawn = Combatant;
			}
		}
	}

	// 3. 찾은 시간만큼 모두를 "타임 워프" 시킴 (시간 흘리기)
	if (NextPawn && MinTimeNeeded < MAX_FLT)
	{
		// [핵심] 계산된 시간만큼 모두의 게이지를 증가시킴
		// + 0.001f는 부동소수점 오차로 9999.99가 되어 턴을 못 잡는 억울한 상황 방지용
		AdvanceAllActionValues(AllCombatants, MinTimeNeeded + 0.001f);

		return NextPawn;
	}

	return nullptr;
}

TArray<ACombatPawn*> UTurnSchedulerComponent::GetPredictedTurnOrder(int32 MaxPredictionCount) const
{
	TArray<ACombatPawn*> PredictedOrder;
	if (!BattleManagerRef) return PredictedOrder;

	AddTurnStackToPrediction(PredictedOrder, BattleManagerRef->GetTurnStack(), MaxPredictionCount);
	if (PredictedOrder.Num() >= MaxPredictionCount)
	{
		return PredictedOrder;
	}

	TArray<FSimulatedPawnData> SimPawns = CreateSimulationData(BattleManagerRef->GetAllCombatants(), PredictedOrder);
	RunTurnPredictionSimulation(PredictedOrder, SimPawns, MaxPredictionCount);

	return PredictedOrder;
}

TArray<ACombatPawn*> UTurnSchedulerComponent::FindReadyCombatants(const TArray<ACombatPawn*>& AllCombatants) const
{
	TArray<ACombatPawn*> ReadyCombatants;
	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated && Combatant->GetBattleTurnComponent()->IsReadyForTurn())
		{
			ReadyCombatants.Add(Combatant);
		}
	}
	return ReadyCombatants;
}

void UTurnSchedulerComponent::SortCombatantsByPriority(TArray<ACombatPawn*>& Combatants) const
{
	Combatants.Sort([](const ACombatPawn& A, const ACombatPawn& B) {
		const float SpeedA = A.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
		const float SpeedB = B.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
		if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
		{
			return SpeedA > SpeedB;
		}
		const EFaction FactionA = A.GetFaction();
		const EFaction FactionB = B.GetFaction();
		if (FactionA != FactionB)
		{
			return FactionA == EFaction::Player;
		}
		return A.GetUniqueID() < B.GetUniqueID();
		});
}

UTurnSchedulerComponent::FTimeToActData UTurnSchedulerComponent::CalculateMinTimeToAct(const TArray<ACombatPawn*>& AllCombatants) const
{
	FTimeToActData Result;
	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			UBattleTurnComponent* TurnComp = Combatant->GetBattleTurnComponent();
			UAttributesComponent* AttrComp = Combatant->GetAttributesComponent();
			if (TurnComp && AttrComp)
			{
				const float Speed = AttrComp->GetCurrentStats().fMovementSpeed;
				if (Speed <= 0) continue;
				const float TimeToReachThreshold = (TurnComp->GetActionThreshold() - TurnComp->GetActionValue()) / Speed;
				if (TimeToReachThreshold >= 0 && TimeToReachThreshold < Result.MinTimeToAct)
				{
					Result.MinTimeToAct = TimeToReachThreshold;
					Result.NextCombatant = Combatant;
				}
			}
		}
	}
	return Result;
}

void UTurnSchedulerComponent::AdvanceAllActionValues(const TArray<ACombatPawn*>& AllCombatants, float TimeDelta)
{
	if (TimeDelta <= 0) return;
	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			Combatant->GetBattleTurnComponent()->AdvanceActionValue(TimeDelta);
		}
	}
}

void UTurnSchedulerComponent::AddTurnStackToPrediction(TArray<ACombatPawn*>& OutPredictedOrder, const TArray<FTurnContext>& TurnStack, int32 MaxPredictionCount) const
{
	for (int32 i = TurnStack.Num() - 1; i >= 0; --i)
	{
		if (OutPredictedOrder.Num() >= MaxPredictionCount) break;
		if (TurnStack[i].Combatant)
		{
			OutPredictedOrder.Add(TurnStack[i].Combatant);
		}
	}
}

TArray<UTurnSchedulerComponent::FSimulatedPawnData> UTurnSchedulerComponent::CreateSimulationData(const TArray<ACombatPawn*>& AllCombatants, const TArray<ACombatPawn*>& InitialPrediction) const
{
	TArray<FSimulatedPawnData> SimPawns;
	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			FSimulatedPawnData SimData(Combatant);
			if (InitialPrediction.Contains(Combatant))
			{
				SimData.SimulatedActionValue = 0.f;
			}
			SimPawns.Add(SimData);
		}
	}
	return SimPawns;
}

void UTurnSchedulerComponent::RunTurnPredictionSimulation(TArray<ACombatPawn*>& OutPredictedOrder, TArray<FSimulatedPawnData>& SimPawns, int32 MaxPredictionCount) const
{
	int32 SafeGuardLoop = 0; // 무한 루프 방지용

	while (OutPredictedOrder.Num() < MaxPredictionCount)
	{
		// 1. 가장 빨리 턴 잡을 애가 걸릴 시간 계산
		float MinTimeToAct = CalculateMinTimeToAct_Simulated(SimPawns);

		// 더 이상 계산할 수 없거나 너무 먼 미래라면 중단
		if (MinTimeToAct >= MAX_FLT || SafeGuardLoop++ > 100) break;

		// 2. 그 시간만큼 모두를 전진 (Time Warp)
		// 오차 방지를 위해 아주 미세한 값을 더해줌 (실제 로직과 동일)
		float TimeStep = MinTimeToAct + 0.001f;

		for (FSimulatedPawnData& SimPawn : SimPawns)
		{
			SimPawn.SimulatedActionValue += SimPawn.MovementSpeed * TimeStep;
		}

		// 3. 목표 도달한 녀석들 찾기 (10000 넘은 애들)
		TArray<FSimulatedPawnData*> ReadyPawns = FindAndSortReadyPawns_Simulated(SimPawns);

		// 4. 결과 목록에 추가하고 게이지 차감
		for (FSimulatedPawnData* ReadyPawnPtr : ReadyPawns)
		{
			if (OutPredictedOrder.Num() >= MaxPredictionCount) break;

			ACombatPawn* NextPawn = ReadyPawnPtr->Pawn.Get();
			if (NextPawn)
			{
				OutPredictedOrder.Add(NextPawn);

				// [중요] 실제 로직과 똑같이 '차감' 방식으로 처리
				ReadyPawnPtr->SimulatedActionValue -= 10000.f;
			}
		}

		// (참고) 만약 ReadyPawns가 없다면 루프를 다시 돌아 다음 시간을 계산합니다.
	}
}

float UTurnSchedulerComponent::CalculateMinTimeToAct_Simulated(const TArray<FSimulatedPawnData>& SimPawns) const
{
	float MinTimeToAct = MAX_FLT;

	for (const FSimulatedPawnData& SimPawn : SimPawns)
	{
		// 이미 목표치(10000)를 넘긴 애가 있다면 시간은 0입니다.
		if (SimPawn.SimulatedActionValue >= 10000.f)
		{
			return 0.0f;
		}

		if (SimPawn.MovementSpeed <= 0.f) continue;

		// (목표 - 현재) / 속도
		float TimeNeeded = (10000.f - SimPawn.SimulatedActionValue) / SimPawn.MovementSpeed;

		if (TimeNeeded < MinTimeToAct)
		{
			MinTimeToAct = TimeNeeded;
		}
	}

	return MinTimeToAct;
}

TArray<UTurnSchedulerComponent::FSimulatedPawnData*> UTurnSchedulerComponent::FindAndSortReadyPawns_Simulated(TArray<FSimulatedPawnData>& SimPawns) const
{
	TArray<FSimulatedPawnData*> ReadyPawns;
	for (FSimulatedPawnData& SimPawn : SimPawns)
	{
		if (SimPawn.SimulatedActionValue >= 10000.f - KINDA_SMALL_NUMBER)
		{
			ReadyPawns.Add(&SimPawn);
		}
	}

	if (ReadyPawns.Num() > 0)
	{
		ReadyPawns.Sort([](const FSimulatedPawnData& A, const FSimulatedPawnData& B) {
			if (FMath::IsNearlyEqual(A.MovementSpeed, B.MovementSpeed))
			{
				return A.SimulatedActionValue > B.SimulatedActionValue;
			}
			return A.MovementSpeed > B.MovementSpeed;
			});
	}
	return ReadyPawns;
}