// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SPGAS_DamageCalculation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGAS_DamageCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	USPGAS_DamageCalculation();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
