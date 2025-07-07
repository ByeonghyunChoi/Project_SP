// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "MonsterBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class PROJECT_SP_API UMonsterBase : public UCharacterBase
{
	GENERATED_BODY()

public:
	UMonsterBase();
	virtual void DecideAction() override PURE_VIRTUAL(UMonsterBase::DecideAction,);
	
};
