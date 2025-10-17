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
	TArray<ACombatPawn*> ReadyCombatants = FindReadyCombatants(AllCombatants);

	if (ReadyCombatants.Num() > 0)
	{
		SortCombatantsByPriority(ReadyCombatants);
		return ReadyCombatants[0];
	}

	const FTimeToActData TimeToActData = CalculateMinTimeToAct(AllCombatants);
	if (TimeToActData.NextCombatant)
	{
		AdvanceAllActionValues(AllCombatants, TimeToActData.MinTimeToAct);
		return TimeToActData.NextCombatant;
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
	while (OutPredictedOrder.Num() < MaxPredictionCount && SimPawns.Num() > 0)
	{
		const float MinTimeToAct = CalculateMinTimeToAct_Simulated(SimPawns);
		if (MinTimeToAct >= MAX_FLT) break;

		for (FSimulatedPawnData& SimPawn : SimPawns)
		{
			SimPawn.SimulatedActionValue += SimPawn.MovementSpeed * MinTimeToAct;
		}

		TArray<FSimulatedPawnData*> ReadyPawns = FindAndSortReadyPawns_Simulated(SimPawns);

		for (FSimulatedPawnData* ReadyPawnPtr : ReadyPawns)
		{
			if (OutPredictedOrder.Num() >= MaxPredictionCount) break;

			ACombatPawn* NextPawn = ReadyPawnPtr->Pawn.Get();
			if (NextPawn)
			{
				OutPredictedOrder.Add(NextPawn);
				ReadyPawnPtr->SimulatedActionValue -= 10000.f;
			}
		}
	}
}

float UTurnSchedulerComponent::CalculateMinTimeToAct_Simulated(const TArray<FSimulatedPawnData>& SimPawns) const
{
	float MinTimeToAct = MAX_FLT;
	for (const FSimulatedPawnData& SimPawn : SimPawns)
	{
		const float TimeToReach = SimPawn.GetTimeToReachThreshold(10000.f);
		MinTimeToAct = FMath::Min(MinTimeToAct, FMath::Max(0.f, TimeToReach));
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