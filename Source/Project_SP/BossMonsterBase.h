// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBase.h"
#include "BossMonsterBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UBossMonsterBase : public UMonsterBase
{
	GENERATED_BODY()
public:
	UBossMonsterBase();
	virtual void DecideAction() override;
	
};
