// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_TriggerHitStop.h"
#include "Core/BattleManager.h"

void UTask_TriggerHitStop::ExecuteTask_Implementation()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    // [로그 추가] HitStop 발동 시간
    UE_LOG(LogTemp, Error, TEXT("[Time: %f] !!! TRIGGER HIT STOP !!!"), CurrentTime);

    if (BattleManager)
    {
        // 배틀 매니저에게 "지금 멈춰!"라고 명령
        BattleManager->ActivateHitStop(Duration, TimeDilation);
    }
    FinishTask();
}