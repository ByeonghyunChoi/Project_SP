// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RelicBase.h"
#include "RLC_CompressedIron.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API URLC_CompressedIron : public URelicBase
{
	GENERATED_BODY()

protected:
	virtual void ApplyRelicEffect(AActor* Target) override;
	virtual void RemoveRelicEffect(AActor* Target) override;

	virtual FStatModifiers GetRelicModifiers() const override;
};
