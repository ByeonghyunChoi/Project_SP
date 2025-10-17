// Combat/MonsterAIController.cpp

#include "Combat/MonsterAIController.h"
#include "Character/MonsterCharacter.h"
#include "Component/ActionComponent.h"
#include "Combat/GameAction.h" 
#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h"

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void AMonsterAIController::OnTurnBegan(const TArray<ACombatPawn*>& PotentialTargets)
{
    // 바로 행동하면 너무 기계적이므로, 0.5초 정도 딜레이를 줍니다.
    FTimerHandle TimerHandle;

    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &AMonsterAIController::DecideAction, PotentialTargets);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
}

void AMonsterAIController::DecideAction(TArray<ACombatPawn*> PlayerPawns)
{
    AMonsterCharacter* ControlledMonster = Cast<AMonsterCharacter>(GetPawn());
    if (!ControlledMonster || PlayerPawns.Num() == 0) return;

    UActionComponent* ActionComp = ControlledMonster->GetActionComponent();
    if (!ActionComp || ActionComp->GetGrantedActions().Num() == 0) return;

    // 1. 사용할 액션을 랜덤으로 선택합니다. (나중에 더 복잡한 로직으로 교체 가능)
    const TArray<TObjectPtr<UGameAction>>& Actions = ActionComp->GetGrantedActions();
    const FName ActionIDToUse = Actions[FMath::RandRange(0, Actions.Num() - 1)]->GetActionID();

    // 2. 공격할 타겟을 랜덤으로 선택합니다.
    TArray<ACombatPawn*> Targets;
    Targets.Add(PlayerPawns[FMath::RandRange(0, PlayerPawns.Num() - 1)]);

    // 3. '몸'에게 애니메이션 재생을 지시합니다.
    ControlledMonster->PlayActionMontage(ActionIDToUse);

    // 4. '몸'의 ActionComponent에게 액션 실행을 직접 명령합니다.
    ActionComp->StartActionByID(ControlledMonster, ActionIDToUse, Targets);
}