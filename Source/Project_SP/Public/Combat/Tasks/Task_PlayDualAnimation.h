// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_PlayDualAnimation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_PlayDualAnimation : public UCombatTask
{
	GENERATED_BODY()
	
public:
	virtual void ExecuteTask_Implementation() override;
	virtual bool IsLatent() const override { return true; } // 대기 기능 활성화

protected:
	// 시전자(주로 플레이어)가 재생할 몽타주
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	TObjectPtr<UAnimMontage> InstigatorMontage;

	// 대상(주로 몬스터)이 재생할 몽타주
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	TObjectPtr<UAnimMontage> TargetMontage;

private:
	FTimerHandle TimerHandle_AnimWait;

	// 타이머가 끝나면 호출될 함수
	void OnAnimationsFinished();
};
