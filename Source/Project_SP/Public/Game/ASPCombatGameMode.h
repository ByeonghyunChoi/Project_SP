// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASPCombatGameMode.generated.h"

class ASPBattleCameraActor;
class ASPBattleDirector;

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

	UFUNCTION(BlueprintCallable, Category = "Combat | Flow")
	void EndBattle(bool bPlayerWon);

	TArray<TObjectPtr<AActor>> GetCurrentEnemies();

	UFUNCTION(BlueprintCallable, Category = "Battle | UI")
	void RefreshTurnTimelineUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat | Flow")
	void PlayVictorySequence();

	UFUNCTION(BlueprintCallable, Category = "Combat | Flow")
	void OnCharacterDied(AActor* DeadActor);

	//getter
	FORCEINLINE TObjectPtr<class ASPCombatTurnManager> GetTurnManager() { return TurnManager; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Battle")
	TSubclassOf<class ASPCombatTurnManager> TurnManagerClass;

	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	TObjectPtr<class ASPCombatTurnManager> TurnManager;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Battle")
	TObjectPtr<AActor> CurrentTurnActor;

	UPROPERTY()
	TArray<AActor*> AllParticipants;

	UPROPERTY()
	TSet<AActor*> ReadyParticipants;

	bool bIsBattleInitialized = false;

	UPROPERTY(EditDefaultsOnly, Category = "Battle|TimePower")
	TSubclassOf<class UGameplayEffect> TurnEndTimeCostGE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | ReadyCheck")
	int32 TotalExpectedParticipants = 0;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsCurrentTurnInterrupt = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	UDataTable* CombatRewardDataTable;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsCurrentTurnParry = false;

	// 클래스 할당용 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Battle | Visual")
	TSubclassOf<class ASPBattleCameraActor> CameraManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Battle | Visual")
	TSubclassOf<class ASPBattleDirector> BattleDirectorClass;

	// 생성된 인스턴스 저장용
	UPROPERTY(BlueprintReadOnly, Category = "Battle | Visual")
	TObjectPtr<class ASPBattleCameraActor> ActiveCameraManager;

	UPROPERTY(BlueprintReadOnly, Category = "Battle | Visual")
	TObjectPtr<class ASPBattleDirector> ActiveBattleDirector;

protected:
	void ProcessEndOfTurn();

private:
	FTransform GetSpawnTransformByIndex(int32 Index);

	void InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player);

	void FinalizeBattleSetup();

	void CheckAndStartBattle();

	void ApplyPlayerSavedData();

};
