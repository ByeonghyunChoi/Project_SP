#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CombatPawn.h" 
#include "Data/MonsterData.h"
#include "Core/MyGameInstance.h" 
#include "BattleManager.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Setup UMETA(DisplayName = "전투 준비"),
	InProgress UMETA(DisplayName = "전투 진행 중"),
	PlayerTurn UMETA(DisplayName = "플레이어 턴"),
	EnemyTurn UMETA(DisplayName = "적 턴"),
	ExecutingAction UMETA(DisplayName = "행동 실행 중"),
	Ended UMETA(DisplayName = "전투 종료")
};

UENUM(BlueprintType)
enum class EParryResult : uint8
{
	None,           // 패링 시도 없음
	Success,        // 패링 성공 (약점 일치)
	PartialSuccess  // 부분 성공 (약점 불일치)
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnOrderChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleStateChanged, EBattleState, NewState);

UCLASS()
class PROJECT_SP_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

protected:
	virtual void BeginPlay() override;

	//턴 계산에 사용될 전역 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle Manager")
	float GlobalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Tables")
	UDataTable* CharacterStatsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Tables")
	UDataTable* ActionsDataTable;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "Battle Manager|State")
	EBattleState CurrentBattleState;

	UPROPERTY(BlueprintReadOnly, Category = "Battle Manager|Turn")
	ACombatPawn* CurrentTurnCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle Manager|Combatants")
	TArray<ACombatPawn*> AllCombatants;

	UPROPERTY(BlueprintAssignable, Category = "Battle Manager|Combatants")
	FOnTurnOrderChanged OnTurnOrderChanged;

	UPROPERTY(BlueprintAssignable, Category = "Battle Manager|Events")
	FOnBattleStateChanged OnBattleStateChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle Manager|Turn")
	EParryResult CurrentTurnParryResult;

	UFUNCTION(BlueprintCallable)
	void StartBattle();

	UFUNCTION(BlueprintCallable)
	void EndBattle();

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Turn")
	void ProcessTurn();

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Turn")
	void InitiateTurnFor(ACombatPawn* Target);

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Turn")
	void EndTurn();

	UFUNCTION(BlueprintCallable)
	void AddCombatant(ACombatPawn* NewCombatant);

	UFUNCTION(BlueprintCallable)
	const TArray<ACombatPawn*>& GetAllCombatants() const;

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Utility")
	TArray<ACombatPawn*> GetReadyCombatants() const;

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Utility")
	void AdvanceAllActionValues(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Utility")
	float GetMinTimeToNextTurn() const;

	UFUNCTION(BlueprintCallable, Category = "Battle Manager|Utility")
	bool CheckBattleEndConditions() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle Manager|Utility")
	static bool CombatantSortPredicate(const ACombatPawn* A, const ACombatPawn* B);

	UFUNCTION(BlueprintCallable)
	ACombatPawn* GetCurrentTurnCharacter() const;

	UFUNCTION(BlueprintPure)
	EParryResult GetCurrentTurnParryResult() const { return CurrentTurnParryResult; }

protected:
	UFUNCTION()
	void HandleCombatantActionFinished(ACombatPawn* FinishedPawn);

	UFUNCTION()
	void HandleCombatantDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, ACombatPawn* InstigatorPawn, UDamageType* DamageType);

	UFUNCTION()
	void HandleCombatantHealthChanged(ACombatPawn* CombatPawn, float CurrentHealth);

	UFUNCTION()
	void HandleCombatantTurnStarted(ACombatPawn* TurnPawn);

	UFUNCTION()
	void HandleCombatantTurnEnded(ACombatPawn* TurnPawn);

	UFUNCTION()
	void HandleParryAttempt(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult);

	UFUNCTION()
	void SetCurrentBattleState(EBattleState NewState);
};