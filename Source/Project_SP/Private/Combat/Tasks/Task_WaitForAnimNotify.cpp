// Combat/Tasks/Task_WaitForAnimNotify.cpp

#include "Combat/Tasks/Task_WaitForAnimNotify.h"
#include "Character/CombatPawn.h"
#include "Animation/AnimInstance.h"

void UTask_WaitForAnimNotify::ExecuteTask_Implementation()
{
}

void UTask_WaitForAnimNotify::OnNotifyReceived(FName ReceivedNotifyName)
{
	// BattleManager로부터 받은 신호가 내가 기다리던 이름과 일치하는지 확인합니다.
	if (ReceivedNotifyName == NotifyName)
	{
		// 일치하면 작업을 종료합니다.
		FinishTask();
	}
}