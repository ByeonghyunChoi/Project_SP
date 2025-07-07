// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBase.h"
#include "NormalMonsterBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UNormalMonsterBase : public UMonsterBase
{
	GENERATED_BODY()
	
public:
	UNormalMonsterBase();
	virtual void DecideAction() override;
};
