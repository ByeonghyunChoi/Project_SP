// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MonsterAIController.h"
#include "Character/MonsterCharacter.h"
#include "Component/ActionComponent.h"
#include "Combat/GameAction.h" 
#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h"

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ControlledMonster = Cast<AMonsterCharacter>(InPawn);
}

void AMonsterAIController::OnTurnBegan()
{
    // 바로 행동하면 너무 기계적이므로, 0.5초 정도 딜레이를 줍니다.
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMonsterAIController::DecideAction, 0.5f, false);
}

void AMonsterAIController::DecideAction()
{
    if (!ControlledMonster) return;

    // 1. 사용 가능한 액션 목록 가져오기
    UActionComponent* ActionComp = ControlledMonster->GetActionComponent();
    if (!ActionComp || ActionComp->GetGrantedActions().Num() == 0)
    {
        // 사용할 스킬이 없으면 바로 턴 종료
        ActionComp->EndActiveAction(ControlledMonster);
        return;
    }
    const TArray<TObjectPtr<UGameAction>>& Actions = ActionComp->GetGrantedActions();

    // 2. 사용할 액션 랜덤 선택 (나중에 더 복잡한 로직으로 교체 가능)
    const FName ActionIDToUse = Actions[FMath::RandRange(0, Actions.Num() - 1)]->GetActionID();

    // 3. 공격할 타겟 찾기
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    TArray<ACombatPawn*> PlayerPawns;
    if (BattleManager)
    {
        for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
        {
            if (Combatant && Combatant->GetFaction() == EFaction::Player && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
            {
                PlayerPawns.Add(Combatant);
            }
        }
    }

    if (PlayerPawns.Num() == 0)
    {
        // 공격할 타겟이 없으면 바로 턴 종료
        ActionComp->EndActiveAction(ControlledMonster);
        return;
    }

    // 4. 타겟 랜덤 선택
    TArray<ACombatPawn*> Targets;
    Targets.Add(PlayerPawns[FMath::RandRange(0, PlayerPawns.Num() - 1)]);

    // 5. 애니메이션 재생 및 액션 시작
    ControlledMonster->PlayActionMontage(ActionIDToUse);
    ActionComp->StartActionByID(ControlledMonster, ActionIDToUse, Targets);
}

