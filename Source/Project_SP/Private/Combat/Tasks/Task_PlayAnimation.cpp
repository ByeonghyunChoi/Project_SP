// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_PlayAnimation.h"
#include "Character/CombatPawn.h"

void UTask_PlayAnimation::ExecuteTask_Implementation()
{
	if (Instigator && MontageToPlay)
	{
		// 애니메이션이 끝나면 OnMontageEnded 함수를 호출하도록 예약합니다.
		FOnMontageEnded OnEnded;
		OnEnded.BindUObject(this, &UTask_PlayAnimation::OnMontageEnded);

		Instigator->PlayAnimMontage(MontageToPlay);
		Instigator->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(OnEnded, MontageToPlay);
	}
	else
	{
		// 재생할 애니메이션이 없으면 즉시 작업을 종료합니다.
		FinishTask();
	}
}

void UTask_PlayAnimation::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 애니메이션이 끝나면 시퀀서에게 "내 일은 끝났어!" 라고 보고합니다.
	FinishTask();
}

