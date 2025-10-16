// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_OpenParryWindow.h"
#include "Character/CombatPawn.h"
#include "Component/GameEventComponent.h"
#include "Combat/GameAction.h"
#include "TimerManager.h"

void UTask_OpenParryWindow::ExecuteTask_Implementation()
{
	UGameAction* OwningAction = Cast<UGameAction>(GetOuter());
	if (Instigator && OwningAction && Instigator->GetFaction() == EFaction::Enemy)
	{
		const FActionData& ActionData = OwningAction->GetData();
		if (ActionData.ParryWindowDuration > 0.f)
		{
			if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
			{
				// 1. "패링 창 열림!" 이라고 월드에 방송합니다.
				EventComp->BroadcastParryWindowOpened(Instigator, ActionData.DamageType, ActionData.ParryWindowDuration);

				// 2. 정해진 시간 후에 OnParryWindowTimerEnd 함수를 호출하도록 타이머를 설정합니다.
				if (UWorld* World = GetWorld()) // UObject는 GetWorld()를 직접 호출할 수 있습니다.
				{
					World->GetTimerManager().SetTimer(
						ParryWindowTimerHandle,
						this,
						&UTask_OpenParryWindow::OnParryWindowTimerEnd,
						ActionData.ParryWindowDuration,
						false
					);
				}
				// FinishTask()를 여기서 호출하지 않고, 타이머가 만료될 때까지 기다립니다.
				return;
			}
		}
	}

	// 패링이 불가능한 액션이거나, 조건이 맞지 않으면 즉시 작업을 종료합니다.
	FinishTask();
}

void UTask_OpenParryWindow::OnParryWindowTimerEnd()
{
	if (Instigator)
	{
		if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
		{
			// "패링 창 닫힘"을 방송합니다.
			EventComp->BroadcastParryWindowClosed(Instigator);
		}
	}

	// 이제 패링 창의 열림과 닫힘에 대한 모든 임무가 끝났으므로, 작업을 종료합니다.
	FinishTask();
}
