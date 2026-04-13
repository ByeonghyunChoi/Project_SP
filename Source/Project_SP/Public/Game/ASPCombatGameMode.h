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

	UFUNCTION(BlueprintCallable, Category = "Combat | Flow")
	void EndBattle(bool bPlayerWon);

	TArray<TObjectPtr<AActor>> GetCurrentEnemies();

	UFUNCTION(BlueprintCallable, Category = "Battle | UI")
	void RefreshTurnTimelineUI();

	//���� �¸� �� ȣ���� ���� �Լ�
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat | Flow")
	void PlayVictorySequence();

	//getter
	FORCEINLINE TObjectPtr<class ASPCombatTurnManager> GetTurnManager() { return TurnManager; }

protected:
	// ��������Ʈ���� BP_TurnManager�� �����ؾ� ��
	UPROPERTY(EditDefaultsOnly, Category = "Battle")
	TSubclassOf<class ASPCombatTurnManager> TurnManagerClass;

	// ���� ������ �Ŵ���
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	TObjectPtr<class ASPCombatTurnManager> TurnManager;

	// ���� �� ����
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Battle")
	TObjectPtr<AActor> CurrentTurnActor;

	// ���� ������ ���
	UPROPERTY()
	TArray<AActor*> AllParticipants;

	//���� �غ� �Ϸ�� ������ ��� 
	UPROPERTY()
	TSet<AActor*> ReadyParticipants;

	bool bIsBattleInitialized = false;

	UPROPERTY(EditDefaultsOnly, Category = "Battle|TimePower")
	TSubclassOf<class UGameplayEffect> TurnEndTimeCostGE;

	//���� ��ü ���� ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | ReadyCheck")
	int32 TotalExpectedParticipants = 0;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsCurrentTurnInterrupt = false;

	// ���� ���� ������ ���̺� (�����Ϳ��� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	UDataTable* CombatRewardDataTable;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsCurrentTurnParry = false;

protected:
	void ProcessEndOfTurn();

private:
	FTransform GetSpawnTransformByIndex(int32 Index);

	void InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player);

	void FinalizeBattleSetup();

	void CheckAndStartBattle();

	//������ ���� �Լ�
	void ApplyPlayerSavedData();

};
