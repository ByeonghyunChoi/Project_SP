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

	FinishTask();
}

