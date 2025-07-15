#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleManager.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Setup UMETA(DisplayName = "전투 준비"),
	InProgress UMETA(DisplayName = "전투 진행 중"),
	PlayerTurn UMETA(DisplayName = "플레이어 턴"),
	EnemyTurn UMETA(DisplayName = "적 턴"),
	WaitForPlayerInput UMETA(DisplayName = "플레이어 입력 대기"), 
	Ended UMETA(DisplayName = "전투 종료")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnOrderChanged);

UCLASS()
class PROJECT_SP_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<ACombatPawn*> AllCombatants;

	UPROPERTY()
	ACombatPawn* CurrentTurnCharacter;

	UPROPERTY()
	EBattleState CurrentBattleState;

	UPROPERTY()
	float GlobalTime;

	void ProcessTurn();
	void InitiateTurnFor(ACombatPawn* Target);
	void AdvanceAllActionValues(float DeltaTime);
	TArray<ACombatPawn*> GetReadyCombatants() const;
	float GetMinTimeToNextTurn() const;
	bool CheckBattleEndConditions() const;
	static bool CombatantSortPredicate(const ACombatPawn& A, const ACombatPawn& B);

public:
	UPROPERTY(BlueprintAssignable)
	FOnTurnOrderChanged OnTurnOrderChanged;

	UFUNCTION(BlueprintCallable)
	void StartBattle(const TArray<ACombatPawn*>& InitialCombatants);

	UFUNCTION(BlueprintCallable)
	void EndTurn();

	UFUNCTION(BlueprintCallable)
	void EndBattle();

	UFUNCTION(BlueprintCallable)
	void AddCombatant(ACombatPawn* NewCombatant);

	UFUNCTION(BlueprintCallable)
	const TArray<ACombatPawn*>& GetAllCombatants() const;

	UFUNCTION(BlueprintCallable)
	ACombatPawn* GetCurrentTurnCharacter() const;
};
