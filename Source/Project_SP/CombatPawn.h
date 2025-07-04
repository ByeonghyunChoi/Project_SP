// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.h"
#include "CombatPawn.generated.h"

UCLASS(Abstract)
class PROJECT_SP_API ACombatPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatPawn();
	virtual UCharacterBase* GetCombatData() const PURE_VIRTUAL(ACombatPawn::GetCombatData, return nullptr;);

};
