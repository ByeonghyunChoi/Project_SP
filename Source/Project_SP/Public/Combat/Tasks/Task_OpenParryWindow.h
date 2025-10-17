// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_OpenParryWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_OpenParryWindow : public UCombatTask
{
	GENERATED_BODY()
	
public:
	virtual void ExecuteTask_Implementation() override;

private:
	// 패링 가능 시간이 만료되었을 때 타이머가 호출할 함수
	void OnParryWindowTimerEnd();

	// 타이머를 제어하기 위한 핸들
	FTimerHandle ParryWindowTimerHandle;
};
