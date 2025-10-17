// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_PlayCameraShot.h"
#include "Core/BattleManager.h"
#include "Component/CombatCameraComponent.h"
#include "Character/CombatPawn.h"

void UTask_PlayCameraShot::ExecuteTask_Implementation()
{
	if (BattleManager && !ShotName.IsNone())
	{
		if (UCombatCameraComponent* CameraComp = BattleManager->GetCameraComponent())
		{
			// BattleManager의 카메라 컴포넌트를 통해 Shot 재생을 명령합니다.
			CameraComp->PlayShot(ShotName, Instigator, Targets.Num() > 0 ? Targets[0] : nullptr);
		}
	}

	// 카메라 샷 재생 '명령'은 즉시 끝나므로, 바로 작업을 종료하고 다음 작업으로 넘어갑니다.
	// (실제 카메라 이동은 CombatCameraComponent의 Tick에서 비동기로 이루어집니다.)
	FinishTask();
}

