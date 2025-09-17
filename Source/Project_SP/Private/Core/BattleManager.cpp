#include "Core/BattleManager.h"
#include "Combat/CombatPawn.h"
#include "Combat/BattleTurnComponent.h"
#include "Combat/AttributesComponent.h"
#include "Event/GameEventComponent.h"
#include "Kismet/GameplayStatics.h"


ABattleManager::ABattleManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABattleManager::BeginPlay()
{
    Super::BeginPlay();
}

void ABattleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentBattleState == EBattleState::InProgress)
    {
        ProcessTurnFlow(DeltaTime);
    }
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
    AllCombatants.Empty();
    AllCombatants.Append(PlayerParty);
    AllCombatants.Append(EnemyParty);

    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->GetGameEventComponent())
        {
            // 각 전투원의 이벤트에 핸들러 함수들을 바인딩합니다.
            Combatant->GetGameEventComponent()->OnActionExecutionFinished.AddDynamic(this, &ABattleManager::HandleActionFinished);
            // TODO: OnInterruptRequest 델리게이트를 GameEventComponent에 만들고 여기에 바인딩
            // Combatant->GetGameEventComponent()->OnInterruptRequest.AddDynamic(this, &ABattleManager::HandleInterruptRequest);
            Combatant->GetAttributesComponent()->OnHealthDepleted.AddDynamic(this, &ABattleManager::HandleCombatantDied);
        }
    }
    CurrentBattleState = EBattleState::InProgress;
}

void ABattleManager::ProcessTurnFlow(float DeltaTime)
{
    // 스택이 비어 있을 때만 다음 턴을 찾습니다.
    if (TurnStack.IsEmpty())
    {
        TArray<ACombatPawn*> ReadyCombatants;
        for (ACombatPawn* Combatant : AllCombatants)
        {
            if (Combatant &&
                Combatant->GetCombatPawnState() != ECombatPawnState::Defeated &&
                Combatant->GetBattleTurnComponent()->IsReadyForTurn())
            {
                ReadyCombatants.Add(Combatant);
            }
        }

        if (ReadyCombatants.Num() > 0)
        {
            // TODO: 속도 등에 따라 우선순위 정렬
            PushAndStartTurn(ReadyCombatants[0], ETurnType::Normal);
        }
        else
        {
            AdvanceAllActionValues(DeltaTime);
        }
    }
}

void ABattleManager::PushAndStartTurn(ACombatPawn* Combatant, ETurnType Type)
{
    if (!Combatant || Combatant->GetCombatPawnState() == ECombatPawnState::Defeated) return;

    TurnStack.Emplace(Combatant, Type);
    Combatant->GetBattleTurnComponent()->StartTurn();
    Combatant->OnTurnBegin();
}

void ABattleManager::EndCurrentTurn()
{
    if (TurnStack.IsEmpty()) return;

    ACombatPawn* EndedTurnCombatant = TurnStack.Last().Combatant;
    TurnStack.Pop();

    EndedTurnCombatant->GetBattleTurnComponent()->EndTurn();

    // TODO: 전투 종료 조건 확인 (CheckBattleEndConditions)
}

ACombatPawn* ABattleManager::GetCurrentTurnCharacter() const
{
    return TurnStack.IsEmpty() ? nullptr : TurnStack.Last().Combatant;
}

void ABattleManager::HandleActionFinished(ACombatPawn* FinishedPawn)
{
    if (GetCurrentTurnCharacter() == FinishedPawn)
    {
        EndCurrentTurn();
    }
}

void ABattleManager::HandleInterruptRequest(ACombatPawn* InInstigator)
{
    UE_LOG(LogTemp, Warning, TEXT("Interrupt Turn Requested by %s!"), *InInstigator->GetName());
    PushAndStartTurn(InInstigator, ETurnType::Interrupt);
}

void ABattleManager::HandleCombatantDied(AActor* InInstigator)
{
    ACombatPawn* DeadPawn = Cast<ACombatPawn>(InInstigator);
    if (DeadPawn)
    {
        // 전투원 목록에서 제거하거나, 전투 불능 상태로 만듭니다.
        // AllCombatants.Remove(DeadPawn);
    }
}

void ABattleManager::AdvanceAllActionValues(float DeltaTime)
{
    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            Combatant->GetBattleTurnComponent()->AdvanceActionValue(DeltaTime);
        }
    }
}

void ABattleManager::EndBattle()
{
    CurrentBattleState = EBattleState::Ended;
    TurnStack.Empty();
    // TODO: 전투 종료 처리 로직 (결과 창 표시, 필드로 전환 등)
}