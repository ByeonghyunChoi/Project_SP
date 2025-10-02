// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

class AMonsterCharacter;
class ACombatPawn;

UCLASS()
class PROJECT_SP_API AMonsterAIController : public AAIController
{
    GENERATED_BODY()

public:
    // AMonsterCharacter가 턴 시작 시 호출하는 함수
    void OnTurnBegan(const TArray<ACombatPawn*>& PotentialTargets);

protected:
    virtual void OnPossess(APawn* InPawn) override;

private:
    // 실제 행동과 타겟을 결정하는 AI 로직
    void DecideAction(TArray<ACombatPawn*> PlayerPawns);
};
