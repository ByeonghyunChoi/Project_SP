// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_ApplyStatusRandom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_ApplyStatusRandom : public UCombatTask
{
	GENERATED_BODY()
public:
	virtual void ExecuteTask_Implementation() override;

protected:
	// 적용할 상태 이상 ID (예: Poison)
	UPROPERTY(EditAnywhere, Category = "Status")
	FName StatusEffectID;
	
};
