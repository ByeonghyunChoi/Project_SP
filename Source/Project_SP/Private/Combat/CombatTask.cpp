// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatTask.h"

void UCombatTask::Initialize(ABattleManager* InBattleManager, ACombatPawn* InInstigator, const TArray<ACombatPawn*>& InTargets)
{
    BattleManager = InBattleManager;
    Instigator = InInstigator;
    Targets = InTargets;
}

void UCombatTask::ExecuteTask_Implementation()
{
    FinishTask();
}

void UCombatTask::FinishTask()
{
    if (OnTaskFinished.IsBound())
    {
        OnTaskFinished.Broadcast();
    }
}