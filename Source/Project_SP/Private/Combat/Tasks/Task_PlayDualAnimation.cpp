// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_PlayDualAnimation.h"
#include "Character/CombatPawn.h"
#include "TimerManager.h"


void UTask_PlayDualAnimation::ExecuteTask_Implementation()
{
    // Instigator(시전자)는 필수입니다.
    if (!Instigator)
    {
        UE_LOG(LogTemp, Warning, TEXT("DualAnim Task: Instigator is missing!"));
        FinishTask();
        return;
    }

    float Duration1 = 0.0f;
    float Duration2 = 0.0f;

    // 1. 시전자(예: 플레이어) 애니메이션 재생
    if (InstigatorMontage)
    {
        Duration1 = Instigator->PlayAnimMontage(InstigatorMontage);
    }

    // 2. 대상(예: 몬스터) 애니메이션 재생
    // Targets 배열의 0번째 요소가 패링 당한 몬스터입니다.
    if (Targets.IsValidIndex(0) && Targets[0])
    {
        if (TargetMontage)
        {
            Duration2 = Targets[0]->PlayAnimMontage(TargetMontage);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DualAnim Task: Target is missing! Only playing Instigator anim."));
    }

    // 3. 둘 중 더 긴 시간만큼 대기 (Latent Action)
    const float WaitTime = FMath::Max(Duration1, Duration2);

    if (WaitTime > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            // 가장 긴 애니메이션이 끝나는 시간에 맞춰 FinishTask 호출 예약
            World->GetTimerManager().SetTimer(
                TimerHandle_AnimWait,
                this,
                &UTask_PlayDualAnimation::OnAnimationsFinished,
                WaitTime,
                false
            );
        }
    }
    else
    {
        // 재생된 애니메이션이 없거나 길이가 0이면 즉시 종료
        FinishTask();
    }
}

void UTask_PlayDualAnimation::OnAnimationsFinished()
{
    // 타이머 종료 -> 태스크 완료 알림 -> BattleManager가 다음 태스크(몬스터 복귀 등) 실행
    FinishTask();
}
