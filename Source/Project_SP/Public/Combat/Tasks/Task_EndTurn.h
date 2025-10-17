// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_EndTurn.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_EndTurn : public UCombatTask
{
	GENERATED_BODY()

public:
	virtual void ExecuteTask_Implementation() override;
};
