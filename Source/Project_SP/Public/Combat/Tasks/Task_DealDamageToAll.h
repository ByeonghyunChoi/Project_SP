// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Tasks/Task_DealDamage.h"
#include "Task_DealDamageToAll.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_DealDamageToAll : public UTask_DealDamage
{
	GENERATED_BODY()
public:
	virtual void ExecuteTask_Implementation() override;
};
