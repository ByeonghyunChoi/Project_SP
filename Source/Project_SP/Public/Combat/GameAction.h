// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/ActionData.h"
#include "GameAction.generated.h"

class ACombatPawn;
class ABattleManager;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGameAction : public UObject
{
	GENERATED_BODY()
public:
	UGameAction();



	UFUNCTION(BlueprintCallable, Category = "Game Action")
	void ExecuteAction(
		ACombatPawn* Instigator,
		FActionData ActionData,
		ABattleManager* BattleManagerRef,
		ACombatPawn* TargetPawn,
		const TArray<ACombatPawn*>& TargetPawns);

	UFUNCTION(BlueprintCallable, Category = "Game Action")
	bool HasEnoughCost(ACombatPawn* Instigator, const FActionData& ActionData) const;

};
