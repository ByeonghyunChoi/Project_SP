#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"
#include "Component/GameEventComponent.h"
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
            Combatant->GetGameEventComponent()->OnInterruptRequest.AddDynamic(this, &ABattleManager::HandleInterruptRequest);
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
            //준비된 캐릭터들을 속도(내림차순)에 따라 정렬합니다.
            ReadyCombatants.Sort([](const ACombatPawn& A, const ACombatPawn& B) {

                // 1. 속도 비교 (내림차순)
                const float SpeedA = A.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
                const float SpeedB = B.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
                if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
                {
                    return SpeedA > SpeedB;
                }

                // 2. 속도가 같으면 진영 비교 (플레이어 우선)
                const EFaction FactionA = A.GetFaction();
                const EFaction FactionB = B.GetFaction();
                if (FactionA != FactionB)
                {
                    // A가 플레이어면 true가 되어 A가 앞으로, B가 플레이어면 false가 되어 B가 앞으로 정렬됨
                    return FactionA == EFaction::Player;
                }

                // 3. 속도와 진영이 모두 같으면 먼저 생성된 액터 우선
                return A.GetUniqueID() < B.GetUniqueID();
                });
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