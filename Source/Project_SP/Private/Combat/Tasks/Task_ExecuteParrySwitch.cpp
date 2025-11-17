
#include "Combat/Tasks/Task_ExecuteParrySwitch.h"
#include "Core/BattleManager.h"

void UTask_ExecuteParrySwitch::ExecuteTask_Implementation()
{
    // Instigator: 공격했던 몬스터 / Targets[0]: 패링한 플레이어
    if (BattleManager && Instigator && Targets.IsValidIndex(0))
    {
        BattleManager->FinalizeParryTurnSwitch(Instigator, Targets[0]);
    }

    FinishTask();
}