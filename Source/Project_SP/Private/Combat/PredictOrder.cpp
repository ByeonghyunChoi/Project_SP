// Combat/PredictOrder.cpp

#include "Combat/PredictOrder.h"
#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"

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

    // --- 1단계: 턴 스택 처리 ---
    const TArray<FTurnContext>& TurnStack = BattleManagerRef->GetTurnStack();
    for (int32 i = TurnStack.Num() - 1; i >= 0; --i)
    {
        if (TurnStack[i].Combatant)
        {
            PredictedOrder.Add(TurnStack[i].Combatant);
        }
    }
    if (PredictedOrder.Num() >= MaxPredictionCount)
    {
        PredictedOrder.SetNum(MaxPredictionCount);
        return PredictedOrder;
    }

    // --- 2단계: 시뮬레이션 준비 ---
    TArray<FSimulatedPawnData> SimPawns;
    for (ACombatPawn* Combatant : BattleManagerRef->GetAllCombatants())
    {
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            FSimulatedPawnData SimData(Combatant);
            if (PredictedOrder.Contains(Combatant))
            {
                SimData.SimulatedActionValue = 0.f;
            }
            SimPawns.Add(SimData);
        }
    }

    // --- 시뮬레이션 루프 ---
    while (PredictedOrder.Num() < MaxPredictionCount && SimPawns.Num() > 0)
    {
        // 1. 최소 시간 계산
        float MinTimeToAct = MAX_FLT;
        for (const FSimulatedPawnData& SimPawn : SimPawns)
        {
            const float TimeToReach = SimPawn.GetTimeToReachThreshold(10000.f);
            MinTimeToAct = FMath::Min(MinTimeToAct, FMath::Max(0.f, TimeToReach));
        }

        if (MinTimeToAct >= MAX_FLT) break;

        // 2. 시간 진행
        for (FSimulatedPawnData& SimPawn : SimPawns)
        {
            SimPawn.SimulatedActionValue += SimPawn.MovementSpeed * MinTimeToAct;
        }

        // 3. 준비된 캐릭터 찾기
        TArray<FSimulatedPawnData*> ReadyPawns;
        for (FSimulatedPawnData& SimPawn : SimPawns)
        {
            if (SimPawn.SimulatedActionValue >= 10000.f - KINDA_SMALL_NUMBER)
            {
                ReadyPawns.Add(&SimPawn);
            }
        }

        if (ReadyPawns.Num() == 0) continue;

        // --- 이 부분을 수정합니다 ---
        // 4. 준비된 캐릭터들을 우선순위(속도 등)에 따라 정렬
        ReadyPawns.Sort([](const FSimulatedPawnData& A, const FSimulatedPawnData& B) {
            // 속도가 같을 경우를 대비해 ActionValue도 비교
            if (FMath::IsNearlyEqual(A.MovementSpeed, B.MovementSpeed))
            {
                return A.SimulatedActionValue > B.SimulatedActionValue;
            }
            return A.MovementSpeed > B.MovementSpeed;
            });
        // -------------------------

        // 5. 예측 목록에 추가
        for (FSimulatedPawnData* ReadyPawnPtr : ReadyPawns)
        {
            if (PredictedOrder.Num() >= MaxPredictionCount) break;
            ACombatPawn* NextPawn = ReadyPawnPtr->Pawn.Get();
            if (NextPawn)
            {
                PredictedOrder.Add(NextPawn);
                ReadyPawnPtr->SimulatedActionValue -= 10000.f;
            }
        }
    }

    return PredictedOrder;
}