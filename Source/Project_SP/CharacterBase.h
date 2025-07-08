// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterStats.h"
#include "ObjectType.h"
#include "CharacterBase.generated.h"

/**
 * 
 */


UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UCharacterBase : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FCharacterStatsData Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	float fActionGauge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	bool bIsMyTurn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	int32 iTurnOrderIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	EFaction CharacterFaction;

public:

	UCharacterBase();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn")
	float GetActionGauge() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn")
	bool GetIsMyTurn() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn")
	int32 GetTurnOrderIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	const FCharacterStatsData& GetStats() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void UpdateActionGauge(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartTurn();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool IsReadyForTurn();	

	UFUNCTION(BlueprintCallable, Category = "Battle")
	virtual void DecideAction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	EFaction GetFaction() const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetFaction(EFaction inFaction);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStats(const FCharacterStatsData& NewStats);
};
