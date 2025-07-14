// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBase.h"
#include "CombatPawn.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class PROJECT_SP_API AMonsterCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();

	UFUNCTION(BlueprintCallable, Category = "MonsterType")
	void PerformMonsterTurnAction();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Data")
	TSubclassOf<UMonsterBase> MonsterClass;

	UPROPERTY() 
	UMonsterBase* MonsterAIInstance;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
