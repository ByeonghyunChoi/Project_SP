// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_TriggerHitStop.generated.h"

UCLASS()
class PROJECT_SP_API UTask_TriggerHitStop : public UCombatTask
{
    GENERATED_BODY()
public:
    virtual void ExecuteTask_Implementation() override;
    virtual bool IsLatent() const override { return false; } // 즉시 실행

protected:
    UPROPERTY(EditAnywhere, Category = "HitStop")
    float Duration = 0.15f;

    UPROPERTY(EditAnywhere, Category = "HitStop")
    float TimeDilation = 0.001f;
};
