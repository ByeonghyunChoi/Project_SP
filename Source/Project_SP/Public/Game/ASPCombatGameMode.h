// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/Asset/SPMonsterData.h"
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

	UFUNCTION(BlueprintCallable, Category = "Battle | Interrupt")
	void RequestInterrupt(AActor* Requester, int32 Count = 1);

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

	UFUNCTION(BlueprintCallable, Category = "Battle | Round")
	void AdvanceBattleTime(float TimePassed);

	// 전투 도중 몬스터 소환하는 함수
	UFUNCTION(BlueprintCallable, Category = "Battle | Summon")
	class ASPGASMonsterCharacter* SummonMonsterMidBattle(class USPMonsterData* MinionData);

	// 전투 메시지 함수
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Battle | UI")
	void ShowBattleWarningMessage(const FString& WarningText, float Duration = 1.5f);

	//getter
	FORCEINLINE TObjectPtr<class ASPCombatTurnManager> GetTurnManager() { return TurnManager; }

	UFUNCTION(BlueprintPure, Category = "Battle | Round")
	FORCEINLINE float GetRemainingRoundLimit() const { return FMath::Max(0.0f, TimePerRound - PassedTimeInCurrentRound); }

	UFUNCTION(BlueprintPure, Category = "Battle | Round")
	FORCEINLINE int32 GetRemainingRounds() const { return FMath::Max(0, MaxRoundsPerCycle - CurrentRound); }

	UFUNCTION(BlueprintPure, Category = "Battle | Round")
	FORCEINLINE float GetAVToCycleEnd() const
	{
		// 현재 라운드의 남은 시간 + (앞으로 남은 온전한 라운드 수 * 라운드당 시간)
		float FutureRoundsAV = FMath::Max(0, GetRemainingRounds()) * TimePerRound;
		return GetRemainingRoundLimit() + FutureRoundsAV;
	}

	UFUNCTION(BlueprintCallable, Category = "Combat | Flow")
	void StartFirstTurn();

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

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsCurrentTurnParry = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Battle | Flow")
	bool bIsBattleRunning = false;

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

	// 라운드 설정용 변수
	// 현재 라운드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | Round")
	int32 CurrentRound = 1;

	// 현재 라운드에서 누적된 행동 수치 (0 ~ 100)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | Round")
	float PassedTimeInCurrentRound = 0.0f;

	// 한 라운드를 꽉 채우는 행동 수치(시간) (기본 100)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battle | Round")
	float TimePerRound = 150.0f;

	// 전투 사이클당 허용되는 최대 라운드 수 (기본 3)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battle | Round")
	int32 MaxRoundsPerCycle = 3;

	// 라운드 오버 시 지불해야 할 시간의 힘(TP)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battle | Round")
	float PenaltyTPCost = 10.0f;

	// 처치한 몬스터 보상 정보를 모아두는 장부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle | Reward")
	TArray<EMonsterRank> DefeatedMonsterRanks;


protected:
	void ProcessEndOfTurn();

	void ApplyRoundPenalty();

private:
	FTransform GetSpawnTransformByIndex(int32 Index);

	void InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player);

	void FinalizeBattleSetup();

	void CheckAndStartBattle();

	void ApplyPlayerSavedData();

	void HandleBattleActionFinished(AActor* ActionActor);

};