#include "Combat/GameAction.h"
#include "Component/ActionComponent.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"
#include "Component/GameEventComponent.h"
#include "Combat/CombatStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BattleManager.h"
#include "Combat/CombatTask.h"
#include "TimerManager.h"

void UGameAction::Initialize(UActionComponent* InOwningComponent, FName InActionID)
{
    OwningComponent = InOwningComponent;
    ActionID = InActionID;

    // 데이터 테이블에서 ActionID에 해당하는 데이터를 찾아 'Data' 변수에 저장
    if (OwningComponent && OwningComponent->GetActionDataTable())
    {
        const FActionData* FoundRow = OwningComponent->GetActionDataTable()->FindRow<FActionData>(ActionID, TEXT(""));
        if (FoundRow)
        {
            Data = *FoundRow;
        }
    }
}

bool UGameAction::CanStartAction_Implementation(ACombatPawn* Instigator)
{
    if (!Instigator) return false;

    if (OwningComponent && OwningComponent->CooldownMap.Contains(ActionID))
    {
        int32 Remaining = OwningComponent->CooldownMap[ActionID];
        if (Remaining > 0)
        {
            FString Msg = FString::Printf(TEXT("쿨타임 중입니다. 남은 턴: %d"), Remaining);
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, Msg);
            return false;
        }
    }

    // 비용이 0이면 항상 실행 가능
    if (Data.CostSP <= 0)
    {
        return true;
    }

    // 시전자의 AttributesComponent를 가져와 SP가 충분한지 확인
    UAttributesComponent* AttributesComp = Instigator->GetAttributesComponent();
    if (AttributesComp && AttributesComp->GetSkillPoint() >= Data.CostSP)
    {
        return true;
    }
    FString DebugMessage = FString::Printf(TEXT("SP가 부족하여 '%s'을(를) 사용할 수 없습니다."), *Data.DisplayName.ToString());
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMessage);
    return false;
}

void UGameAction::StartAction_Implementation(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets)
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager)
    {
        EndAction(Instigator);
        return;
    }

    TArray<UCombatTask*> TaskInstances;
    for (UCombatTask* TaskTemplate : Tasks)
    {
        if (TaskTemplate)
        {
            UCombatTask* NewTask = DuplicateObject<UCombatTask>(TaskTemplate, this);
            NewTask->Initialize(BattleManager, Instigator, Targets);
            TaskInstances.Add(NewTask);
        }
    }

    BattleManager->QueueUpCombatTasks(TaskInstances);
}

void UGameAction::EndAction(ACombatPawn* Instigator)
{
    if (Instigator && Instigator->GetGameEventComponent())
    {
        // GameEventComponent를 통해 "액션 실행이 끝났다"고 방송함.
        Instigator->GetGameEventComponent()->BroadcastActionExecutionFinished(Instigator);
    }
}

