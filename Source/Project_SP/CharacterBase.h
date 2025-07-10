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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FString CharacterName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	bool bIsMyTurn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	EFaction CharacterFaction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	float fActionValue;
public:

	UCharacterBase();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn")
	bool GetIsMyTurn() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	const FCharacterStatsData& GetStats() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	const FString& GetCharacterName() const;

	// 목적지까지 남은 시간을 계산하여 반환 (TimeLeftToAct)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	float GetTimeLeftToAct() const;

	// fActionValue를 특정 시간만큼 증가시키는 함수 (ABattleManager에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void AdvanceActionValue(float TimeIncrement);

	// 턴을 잡을 준비가 되었는지 (목표 거리에 도달했는지)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	bool IsReadyForTurn() const;

	// GetActionValue() Getter (const float -> float, BlueprintPure 추가)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	float GetActionValue() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartTurn(); // 턴 시작 시 fActionValue 초기화 로직 포함

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	virtual void DecideAction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	EFaction GetFaction() const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetFaction(EFaction inFaction);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStats(const FCharacterStatsData& NewStats);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCharacterName(const FString& NewName);
};
