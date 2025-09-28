#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"
#include "Component/GameEventComponent.h"
#include "Component/CombatCameraComponent.h"
#include "Kismet/GameplayStatics.h"


ABattleManager::ABattleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CameraComponent = CreateDefaultSubobject<UCombatCameraComponent>(TEXT("CameraComponent"));
}

void ABattleManager::BeginPlay()
{
    Super::BeginPlay();
}

void ABattleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
    AllCombatants.Empty();
    AllCombatants.Append(PlayerParty);
    AllCombatants.Append(EnemyParty);

   for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant)
        {
            // --- MODIFIED ---: GameEventComponent와 AttributesComponent 이벤트에 핸들러 함수들을 바인딩합니다.
            if (Combatant->GetGameEventComponent())
            {
                Combatant->GetGameEventComponent()->OnActionExecutionFinished.AddDynamic(this, &ABattleManager::HandleActionFinished);
                Combatant->GetGameEventComponent()->OnInterruptRequest.AddDynamic(this, &ABattleManager::HandleInterruptRequest);
            }
            if (Combatant->GetAttributesComponent())
            {
                // 캐릭터가 죽을 때마다 전투 종료 조건을 확인하도록 바인딩
                Combatant->GetAttributesComponent()->OnHealthDepleted.AddDynamic(this, &ABattleManager::HandleCombatantDied);
            }
        }
    }
    CurrentBattleState = EBattleState::InProgress;

    DecideNextTurn();
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

            ACombatPawn* CurrentTurnPawn = ReadyCombatants[0];
            if (CurrentTurnPawn)
            {
                PushAndStartTurn(CurrentTurnPawn, ETurnType::Normal);
            }
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
    TArray<ACombatPawn*> Targets;
    const EFaction TargetFaction = (Combatant->GetFaction() == EFaction::Player) ? EFaction::Enemy : EFaction::Player;
    for (ACombatPawn* Pawn : AllCombatants)
    {
        if (Pawn && Pawn->GetFaction() == TargetFaction && Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            Targets.Add(Pawn);
        }
    }
    Combatant->OnTurnBegin(Targets);
    OnTurnOrderChanged();
}

void ABattleManager::EndCurrentTurn()
{
    if (TurnStack.IsEmpty()) return;

    ACombatPawn* EndedTurnCombatant = TurnStack.Last().Combatant;
    TurnStack.Pop();

    EndedTurnCombatant->GetBattleTurnComponent()->EndTurn();
    OnTurnOrderChanged();
    DecideNextTurn();
}

void ABattleManager::CheckBattleEndConditions()
{
    if (CurrentBattleState != EBattleState::InProgress) return;

    bool bAllPlayersDefeated = true;
    bool bAllEnemiesDefeated = true;

    for (const ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            if (Combatant->GetFaction() == EFaction::Player)
            {
                bAllPlayersDefeated = false;
            }
            else if (Combatant->GetFaction() == EFaction::Enemy)
            {
                bAllEnemiesDefeated = false;
            }
        }
    }

    if (bAllPlayersDefeated)
    {
        UE_LOG(LogTemp, Warning, TEXT("======= BATTLE ENDED - DEFEAT ======="));
        EndBattle();
        // TODO: 패배 처리 로직 호출 (ex: 게임 오버 창, 필드로 돌아가기)
    }
    else if (bAllEnemiesDefeated)
    {
        UE_LOG(LogTemp, Warning, TEXT("======= BATTLE ENDED - VICTORY ======="));
        EndBattle();
        // TODO: 승리 처리 로직 호출 (ex: 결과 창, 보상 획득)
    }
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
       AllCombatants.Remove(DeadPawn);
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

void ABattleManager::DecideNextTurn()
{
    // 중단 턴 등 스택에 이미 처리할 턴이 있다면 먼저 처리
    if (!TurnStack.IsEmpty())
    {
        const FTurnContext NextTurnContext = TurnStack.Pop();
        ACombatPawn* NextInStack = NextTurnContext.Combatant;
        PushAndStartTurn(NextInStack, NextTurnContext.TurnType);
        return;
    }

    // --- 로직 수정 시작 ---

    // 1. 이미 행동 게이지가 가득 찬 캐릭터가 있는지 확인
    TArray<ACombatPawn*> ReadyCombatants;
    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            if (Combatant->GetBattleTurnComponent()->IsReadyForTurn())
            {
                ReadyCombatants.Add(Combatant);
            }
        }
    }

    // 2. 만약 준비된 캐릭터가 있다면, 그중 가장 우선순위가 높은 캐릭터의 턴을 시작
    if (ReadyCombatants.Num() > 0)
    {
        // 준비된 캐릭터들을 속도 > 진영 순으로 정렬
        ReadyCombatants.Sort([](const ACombatPawn& A, const ACombatPawn& B) {
            // ... (이전에 사용했던 Sort 로직과 동일)
            const float SpeedA = A.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
            const float SpeedB = B.GetAttributesComponent()->GetCurrentStats().fMovementSpeed;
            if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
            {
                return SpeedA > SpeedB;
            }
            return A.GetFaction() == EFaction::Player; // 간단한 예시
            });

        PushAndStartTurn(ReadyCombatants[0], ETurnType::Normal);
        return; // 턴을 시작했으므로 함수 종료
    }

    // 3. 준비된 캐릭터가 아무도 없다면, 이제 누가 가장 먼저 준비될지 계산
    ACombatPawn* NextCombatant = nullptr;
    float MinTimeToAct = MAX_FLT;

    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
        {
            // ... (기존의 TimeToReachThreshold 계산 로직은 동일)
            UBattleTurnComponent* TurnComp = Combatant->GetBattleTurnComponent();
            UAttributesComponent* AttrComp = Combatant->GetAttributesComponent();
            if (TurnComp && AttrComp)
            {
                const float Speed = AttrComp->GetCurrentStats().fMovementSpeed;
                if (Speed <= 0) continue;

                const float TimeToReachThreshold = (TurnComp->GetActionThreshold() - TurnComp->GetActionValue()) / Speed;

                // 음수 시간은 무시 (이미 준비된 캐릭터는 위에서 처리했으므로)
                if (TimeToReachThreshold >= 0 && TimeToReachThreshold < MinTimeToAct)
                {
                    MinTimeToAct = TimeToReachThreshold;
                    NextCombatant = Combatant;
                }
            }
        }
    }

    // 4. 다음 턴 주자를 찾았다면, 시간 경과 및 턴 시작
    if (NextCombatant)
    {
        // 찾은 시간만큼 모든 캐릭터의 게이지를 증가
        for (ACombatPawn* Combatant : AllCombatants)
        {
            if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
            {
                Combatant->GetBattleTurnComponent()->AdvanceActionValue(MinTimeToAct);
            }
        }

        PushAndStartTurn(NextCombatant, ETurnType::Normal);
    }
}

void ABattleManager::EndBattle()
{
    CurrentBattleState = EBattleState::Ended;
    TurnStack.Empty();
}