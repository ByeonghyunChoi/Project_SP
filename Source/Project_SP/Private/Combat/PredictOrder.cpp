// Combat/PredictOrder.cpp

#include "Combat/PredictOrder.h"
#include "Core/BattleManager.h"
#include "Combat/CombatPawn.h"
#include "Combat/BattleTurnComponent.h"
#include "Combat/AttributesComponent.h"

// 턴 예측 시뮬레이션을 위한 임시 데이터 구조체
struct FSimulatedPawnData
{
    TWeakObjectPtr<ACombatPawn> Pawn;
    float SimulatedActionValue;
    float MovementSpeed;

    FSimulatedPawnData(ACombatPawn* InPawn)
    {
        Pawn = InPawn;
        if (InPawn)
        {
            SimulatedActionValue = InPawn->GetBattleTurnComponent()->GetActionValue();
            MovementSpeed = InPawn->GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
        }
    }

    float GetTimeToReachThreshold(float Threshold) const
    {
        if (MovementSpeed <= 0.f)
        {
            return MAX_FLT;
        }
        return (Threshold - SimulatedActionValue) / MovementSpeed;
    }
};

UPredictOrder::UPredictOrder()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPredictOrder::BeginPlay()
{
    Super::BeginPlay();
    // 이 컴포넌트는 BattleManager에 부착되어야 하므로, 소유자를 BattleManager로 캐스팅합니다.
    BattleManagerRef = Cast<ABattleManager>(GetOwner());
    if (!BattleManagerRef)
    {
        UE_LOG(LogTemp, Error, TEXT("PredictOrderComponent는 ABattleManager 액터에만 부착해야 합니다!"));
    }
}

TArray<ACombatPawn*> UPredictOrder::GetPredictedTurnOrder(int32 MaxPredictionCount) const
{
    TArray<ACombatPawn*> PredictedOrder;
    if (!BattleManagerRef) return PredictedOrder;

    // --- 1단계: "확정된 현재" - 턴 스택에 있는 캐릭터들을 먼저 추가 ---
    const TArray<FTurnContext>& TurnStack = BattleManagerRef->GetTurnStack();
    // 스택의 맨 위(Top)가 현재 턴이므로, 역순으로 순회하여 예측 목록에 추가합니다.
    for (int32 i = TurnStack.Num() - 1; i >= 0; --i)
    {
        if (TurnStack[i].Combatant)
        {
            PredictedOrder.Add(TurnStack[i].Combatant);
        }
    }

    // 예측 목록이 꽉 찼으면 여기서 바로 반환
    if (PredictedOrder.Num() >= MaxPredictionCount)
    {
        PredictedOrder.SetNum(MaxPredictionCount);
        return PredictedOrder;
    }

    // --- 2단계: "예측된 미래" - 행동 게이지 시뮬레이션 ---

    TArray<FSimulatedPawnData> SimPawns;
    for (ACombatPawn* Combatant : BattleManagerRef->GetAllCombatants())
    {
        // 살아있고, 아직 예측 목록에 없는 캐릭터만 시뮬레이션 대상으로 추가
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated && !PredictedOrder.Contains(Combatant))
        {
            SimPawns.Emplace(Combatant);
        }
    }

    // 시뮬레이션 루프: 예측 목록이 찰 때까지 반복
    while (PredictedOrder.Num() < MaxPredictionCount && SimPawns.Num() > 0)
    {
        // 1. 다음 턴을 잡는 데 걸리는 최소 시간 계산
        float MinTimeToAct = MAX_FLT;
        for (const FSimulatedPawnData& SimPawn : SimPawns)
        {
            MinTimeToAct = FMath::Min(MinTimeToAct, SimPawn.GetTimeToReachThreshold(10000.f));
        }

        if (MinTimeToAct >= MAX_FLT || MinTimeToAct < 0.f)
        {
            break; // 더 이상 진행할 캐릭터가 없으면 종료
        }

        // 2. 모든 시뮬레이션 캐릭터의 행동 게이지를 최소 시간만큼 진행
        for (FSimulatedPawnData& SimPawn : SimPawns)
        {
            SimPawn.SimulatedActionValue += SimPawn.MovementSpeed * MinTimeToAct;
        }

        // 3. 준비가 된 캐릭터들을 모두 찾음
        TArray<FSimulatedPawnData*> ReadyPawns;
        for (FSimulatedPawnData& SimPawn : SimPawns)
        {
            if (SimPawn.SimulatedActionValue >= 10000.f)
            {
                ReadyPawns.Add(&SimPawn);
            }
        }

        if (ReadyPawns.Num() == 0) break;

        // 4. 준비된 캐릭터들을 우선순위(속도 등)에 따라 정렬
        ReadyPawns.Sort([](const FSimulatedPawnData& A, const FSimulatedPawnData& B) {
            return A.MovementSpeed > B.MovementSpeed;
            });

        // 5. 가장 우선순위 높은 캐릭터를 예측 목록에 추가하고, 시뮬레이션 목록에서 제거 준비
        ACombatPawn* NextPawn = ReadyPawns[0]->Pawn.Get();
        if (NextPawn)
        {
            PredictedOrder.Add(NextPawn);
            SimPawns.RemoveAll([NextPawn](const FSimulatedPawnData& SimPawn) {
                return SimPawn.Pawn == NextPawn;
                });
        }
    }

    return PredictedOrder;
}