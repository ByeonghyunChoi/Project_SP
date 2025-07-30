// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatPawn.h"
#include "Combat/MonsterGroupObject.h"
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

    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Monster Group")
    UMonsterGroupObject* CombatMonsterGroup;
public:
    // --- 몬스터 AI 행동 결정 함수 ---
    UFUNCTION(BlueprintCallable, Category = "Monster|AI")
    void DecideAction();

    // --- SelectAction 오버라이드 ---
    virtual void SelectAction(FName ActionID) override;

    // --- 몬스터 전용 이벤트 핸들러 ---
    UFUNCTION()
    void HandleThisMonsterTurnStarted(ACombatPawn* TurnPawn);

    UFUNCTION(BlueprintPure, Category = "Monster Group")
    UMonsterGroupObject* GetCombatMonsterGroup() const;

};
