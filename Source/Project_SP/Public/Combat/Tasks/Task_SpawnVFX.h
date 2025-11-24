// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "NiagaraSystem.h"
#include "Task_SpawnVFX.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_SpawnVFX : public UCombatTask
{
	GENERATED_BODY()

public:
	virtual void ExecuteTask_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Visuals")
	TObjectPtr<UNiagaraSystem> VFXSystem;

	// True: 타겟 위치에 스폰 / False: 시전자 위치에 스폰
	UPROPERTY(EditAnywhere, Category = "Visuals")
	bool bSpawnAtTarget = true;

	// 위치 오프셋 (예: 가슴 높이 Z+100)
	UPROPERTY(EditAnywhere, Category = "Visuals")
	FVector LocationOffset = FVector::ZeroVector;

	// 파티클 크기
	UPROPERTY(EditAnywhere, Category = "Visuals")
	FVector Scale = FVector(1.0f);
	
};
