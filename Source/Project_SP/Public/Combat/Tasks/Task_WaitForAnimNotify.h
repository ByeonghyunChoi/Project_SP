// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_WaitForAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_WaitForAnimNotify : public UCombatTask
{
	GENERATED_BODY()
	
public:
	virtual void ExecuteTask_Implementation() override;

	virtual bool IsLatent() const override { return true; }

	// BattleManager가 신호를 받았을 때 이 함수를 호출합니다.
	virtual void OnNotifyReceived(FName ReceivedNotifyName) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	FName NotifyName;
};
