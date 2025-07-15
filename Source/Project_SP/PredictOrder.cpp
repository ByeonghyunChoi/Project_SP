#include "PredictOrder.h"

UPredictOrder::UPredictOrder()
{
	PrimaryComponentTick.bCanEverTick = false;
	BattleManagerRef = nullptr;
}

void UPredictOrder::BeginPlay()
{
	Super::BeginPlay();
	BattleManagerRef = Cast<ABattleManager>(GetOwner());
	if (BattleManagerRef)
	{
		BattleManagerRef->OnTurnOrderChanged.AddDynamic(this, &UPredictOrder::RequestTurnOrderUpdate);
		UE_LOG(LogTemp, Log, TEXT("UPredictOrder: PredictOrder 연결됨"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPredictOrder: BattleManager에 붙어 있어야 합니다."));
	}
}

TArray<ACombatPawn*> UPredictOrder::GetPredictedTurnOrder() const
{
	TArray<ACombatPawn*> Predicted;
	if (!BattleManagerRef) return Predicted;

	TArray<FSimulatedCombatantData> Simulated;
	for (ACombatPawn* Combatant : BattleManagerRef->GetAllCombatants())
	{
		if (Combatant && Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
		{
			Simulated.Emplace(Combatant);
		}
	}

	const int32 MaxPredict = 4;
	for (int32 Turn = 0; Turn < MaxPredict; ++Turn)
	{
		float MinTime = 99999.f;
		for (const FSimulatedCombatantData& Sim : Simulated)
		{
			if (Sim.bIsAlive && Sim.OriginalSpeed > 0.f)
				MinTime = FMath::Min(MinTime, Sim.GetSimulatedTimeLeftToAct());
		}
		if (MinTime >= 99999.f) break;

		for (FSimulatedCombatantData& Sim : Simulated)
		{
			if (Sim.bIsAlive)
				Sim.SimulatedActionValue += Sim.OriginalSpeed * MinTime;
		}

		TArray<FSimulatedCombatantData*> Ready;
		for (FSimulatedCombatantData& Sim : Simulated)
		{
			if (Sim.bIsAlive && Sim.SimulatedActionValue >= 10000.f)
				Ready.Add(&Sim);
		}

		if (Ready.Num() > 0)
		{
			Ready.Sort(FCompareSimulatedCombatantData());
			if (Ready[0]->OriginalCombatant.IsValid())
			{
				Predicted.Add(Ready[0]->OriginalCombatant.Get());
				Ready[0]->SimulatedActionValue = 0.f;
			}
		}
		else
		{
			break;
		}
	}

	return Predicted;
}

bool UPredictOrder::FCompareSimulatedCombatantData::operator()(const FSimulatedCombatantData& A, const FSimulatedCombatantData& B) const
{
	if (!A.bIsAlive && !B.bIsAlive)
		return A.OriginalCombatant.Get() < B.OriginalCombatant.Get();
	if (!A.bIsAlive) return true;
	if (!B.bIsAlive) return false;

	float TimeA = A.GetSimulatedTimeLeftToAct();
	float TimeB = B.GetSimulatedTimeLeftToAct();
	if (TimeA != TimeB) return TimeA < TimeB;
	if (A.OriginalSpeed != B.OriginalSpeed) return A.OriginalSpeed > B.OriginalSpeed;
	if (A.OriginalFaction == EFaction::Player && B.OriginalFaction != EFaction::Player) return true;
	if (B.OriginalFaction == EFaction::Player && A.OriginalFaction != EFaction::Player) return false;

	return A.OriginalCombatant.Get()->GetUniqueID() < B.OriginalCombatant.Get()->GetUniqueID();
}

void UPredictOrder::RequestTurnOrderUpdate()
{
	if (OnTurnOrderChanged.IsBound())
	{
		OnTurnOrderChanged.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("UPredictOrder: 턴 순서 업데이트 브로드캐스트됨"));
	}
}

bool UPredictOrder::IsMyOnTurnOrderChangedBound() const
{
	return OnTurnOrderChanged.IsBound();
}