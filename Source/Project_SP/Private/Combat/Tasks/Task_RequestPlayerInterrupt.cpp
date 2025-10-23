// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_RequestPlayerInterrupt.h"
#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"


void UTask_RequestPlayerInterrupt::ExecuteTask_Implementation()
{
	if (BattleManager && Instigator) // Instigator는 이 요청을 한 '플레이어'여야 함
	{
		UE_LOG(LogTemp, Log, TEXT("Task_RequestPlayerInterrupt: Requesting interrupt turn for %s"), *Instigator->GetName());
		BattleManager->RequestPlayerInterruptTurn(Instigator);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Task_RequestPlayerInterrupt: BattleManager or Instigator is missing!"));
	}

	// 요청만 하고 즉시 종료
	FinishTask();
}

