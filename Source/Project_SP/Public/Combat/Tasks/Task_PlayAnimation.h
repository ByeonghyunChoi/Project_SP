// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_PlayAnimation.generated.h"

class UAnimMontage;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_PlayAnimation : public UCombatTask
{
	GENERATED_BODY()
	
protected:
	// 이 작업에서 재생할 애니메이션 몽타주입니다.
	// UGameAction 블루프린트의 Tasks 배열에서 직접 설정할 수 있습니다.
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	TObjectPtr<UAnimMontage> MontageToPlay;

public:
	virtual void ExecuteTask_Implementation() override;

private:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
