// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASPCombatGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API AASPCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AASPCombatGameMode();
	virtual void BeginPlay() override;

	void StartTurn(AActor* TurnActor);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn(AActor* TurnActor);

	UFUNCTION(BlueprintCallable, Category = "Battle | ReadyCheck")
	void ReportCharacterReady(AActor* Character);

	void OnCharacterDied(AActor* DeadActor);

	UFUNCTION(BlueprintCallable, Category = "Combat | Flow")
	void EndBattle(bool bPlayerWon);

	TArray<TObjectPtr<AActor>> GetCurrentEnemies();

protected:
	// 블루프린트에서 BP_TurnManager를 지정해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Battle")
	TSubclassOf<class ASPCombatTurnManager> TurnManagerClass;

	// 실제 생성된 매니저
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	TObjectPtr <class ASPCombatTurnManager> TurnManager;

	// 현재 턴 주인
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Battle")
	TObjectPtr<AActor> CurrentTurnActor;

	// 전투 참가자 목록
	UPROPERTY()
	TArray<AActor*> AllParticipants;

	//전투 준비 완료된 참가자 목록 
	UPROPERTY()
	TSet<AActor*> ReadyParticipants;

	bool bIsBattleInitialized = false;

	UPROPERTY(EditDefaultsOnly, Category = "Battle|TimePower")
	TSubclassOf<class UGameplayEffect> TurnEndTimeCostGE;

	//전투 전체 참여 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | ReadyCheck")
	int32 TotalExpectedParticipants = 0;

private:
	FTransform GetSpawnTransformByIndex(int32 Index);

	void InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player);

	void FinalizeBattleSetup();

	void CheckAndStartBattle();

	//데이터 연동 함수
	void ApplyPlayerSavedData();
};
