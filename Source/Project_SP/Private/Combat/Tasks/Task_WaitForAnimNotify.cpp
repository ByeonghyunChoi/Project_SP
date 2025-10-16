// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_WaitForAnimNotify.h"

void UTask_WaitForAnimNotify::ExecuteTask_Implementation()
{
	// 아무것도 하지 않고 BattleManager가 신호를 줄 때까지 기다립니다.
}

void UTask_WaitForAnimNotify::OnNotifyReceived(FName ReceivedNotifyName)
{
	if (ReceivedNotifyName == NotifyName)
	{
		FinishTask();
	}
}
