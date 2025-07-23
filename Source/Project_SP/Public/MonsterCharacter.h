// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatPawn.h"
#include "MonsterGroupObject.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class PROJECT_SP_API AMonsterCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Instanced, Category = "Monster Group")
	UMonsterGroupObject* MonsterGroup;

public:
	UMonsterGroupObject* GetMonsterGroup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Combat")
	void TriggerAttackBlueprintEvent();
};
