// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_PlayCameraShot.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_PlayCameraShot : public UCombatTask
{
	GENERATED_BODY()

protected:
	// 이 작업에서 재생할 카메라 샷의 이름입니다. (ShotDataTable의 Row Name)
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	FName ShotName;

public:
	virtual void ExecuteTask_Implementation() override;
	
};
