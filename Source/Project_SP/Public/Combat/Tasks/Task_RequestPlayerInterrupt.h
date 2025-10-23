// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_RequestPlayerInterrupt.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_RequestPlayerInterrupt : public UCombatTask
{
	GENERATED_BODY()

public:
	/**
	 * BattleManager에게 Instigator(이 태스크를 실행하도록 요청한 플레이어)의
	 * 인터럽트 턴을 시작하도록 요청합니다.
	 */
	virtual void ExecuteTask_Implementation() override;

	// 이 태스크는 즉시 완료됩니다.
	virtual bool IsLatent() const override { return false; }
	
};
