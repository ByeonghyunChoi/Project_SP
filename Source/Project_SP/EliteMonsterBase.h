// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBase.h"
#include "EliteMonsterBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UEliteMonsterBase : public UMonsterBase
{
	GENERATED_BODY()
	
public:
	UEliteMonsterBase();
	virtual void DecideAction() override;
};
