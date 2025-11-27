// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RelicBase.h"
#include "RLC_StrengthAmulet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API URLC_StrengthAmulet : public URelicBase
{
	GENERATED_BODY()
	
protected:
	virtual void ApplyRelicEffect(AActor* Target) override;
	virtual void RemoveRelicEffect(AActor* Target) override;
};
