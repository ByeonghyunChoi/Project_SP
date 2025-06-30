// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBase.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class PROJECT_SP_API AMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MonsterType")
	UMonsterBase* MonsterData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterType")
	TSubclassOf<UMonsterBase> MonsterBaseClass;

	UFUNCTION(BlueprintCallable, Category = "MonsterType")
	void PerformMonsterTurnAction();

};
