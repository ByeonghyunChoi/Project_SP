// Combat/Tasks/Task_PlayAnimation.cpp

#include "Combat/Tasks/Task_PlayAnimation.h"
#include "Character/CombatPawn.h"

void UTask_PlayAnimation::ExecuteTask_Implementation()
{
	if (Instigator && MontageToPlay)
	{
		Instigator->PlayAnimMontage(MontageToPlay);
	}

	// 애니메이션 재생을 '시작'만 시키고, 바로 작업을 종료합니다.
	FinishTask();
}