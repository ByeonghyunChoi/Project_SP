// Core/BattleManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CombatTypes.h"
#include "Data/ActionData.h"
#include "BattleManager.generated.h"

class ACombatPawn;
class UCombatCameraComponent;
class UTurnSchedulerComponent;
class UCombatTask;
class AMyPlayerController;
class UPlayerCombatControlComponent;
enum class EBattleState : uint8;

// 턴 정의 구조체
USTRUCT(BlueprintType)
struct FTurnContext
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ACombatPawn> Combatant;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETurnType TurnType;

	FTurnContext(ACombatPawn* InCombatant = nullptr, ETurnType InType = ETurnType::Normal)
		: Combatant(InCombatant), TurnType(InType) {
	}
};
//턴 정의 끝


UCLASS()
class PROJECT_SP_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTurnSchedulerComponent> TurnScheduler;

	UPROPERTY(VisibleAnywhere, Category = "Battle Flow")
	TArray<FTurnContext> TurnStack;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	EBattleState CurrentBattleState;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	TArray<TObjectPtr<ACombatPawn>> AllCombatants;

	UPROPERTY(EditDefaultsOnly, Category = "Battle Flow")
	float BattleEndDelay = 3.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Battle Flow")
	bool bPlayerWonBattle = false;

	UPROPERTY(EditDefaultsOnly, Category = "Battle Flow|Camera")
	FName StatusEffectFocusShotName = TEXT("Shot_StatusEffectFocus");

	UPROPERTY(EditDefaultsOnly, Category = "Battle Flow|Timing")
	float TurnStartSequenceDelay = 1.5f;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	TArray<TObjectPtr<UPlayerCombatControlComponent>> PlayerControlComponents;

public:
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndBattle();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnTurnOrderChanged();

	UFUNCTION(BlueprintPure, Category = "Battle Flow")
	ACombatPawn* GetCurrentTurnCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Battle Turn")
	const TArray<FTurnContext>& GetTurnStack() const { return TurnStack; }

	const TArray<TObjectPtr<ACombatPawn>>& GetAllCombatants() const { return AllCombatants; }

	FORCEINLINE UCombatCameraComponent* GetCameraComponent() const { return CameraComponent; }

	void QueueUpCombatTasks(const TArray<UCombatTask*>& Tasks);

	void InjectCombatTasks(const TArray<UCombatTask*>& Tasks);

	void ClearTaskQueue();

	void SignalTaskByNotifyName(FName NotifyName);

	void RequestPlayerInterruptTurn(ACombatPawn* PlayerPawn);

protected:
	UFUNCTION()
	void HandleActionFinished(ACombatPawn* FinishedPawn);
	UFUNCTION()
	void HandleInterruptRequest(ACombatPawn* InInstigator);
	UFUNCTION()
	void HandleCombatantDied(AActor* Victim, AActor* InInstigator);
	UFUNCTION()
	void HandleParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult);
	UFUNCTION()
	void HandleDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, EDamageFloaterType DamageType, ACombatPawn* InstigatorPawn);
	UFUNCTION()
	void HandleEnemyParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration);
	UFUNCTION()
	void HandleEnemyParryWindowClosed(ACombatPawn* Attacker);
private:
	void PushAndStartTurn(ACombatPawn* Combatant, ETurnType Type);
	void EndCurrentTurn();
	void CheckBattleEndConditions();
	void DecideAndStartNextTurn();
	void UpdateInputModeForTurn(ACombatPawn* TurnCombatant);

	UPROPERTY()
	TObjectPtr<AMyPlayerController> CachedPlayerController;
	UPROPERTY()
	TArray<TObjectPtr<UCombatTask>> TaskQueue;
	UPROPERTY()
	TObjectPtr<UCombatTask> CurrentTask;
	bool bIsProcessingTask;
	bool bWaitingForTasksToStartTurn = false;
	void ProcessTaskQueue();
	UFUNCTION()
	void OnCurrentTaskFinished();

	void TriggerFieldTransition();

	FTimerHandle BattleEndTimerHandle;

	void OnTurnStartSequenceFinished();

	FTimerHandle TurnStartSequenceTimerHandle;

	void ExecuteParrySequence(ACombatPawn* Attacker, ACombatPawn* Defender);

public:
	void FinalizeParryTurnSwitch(ACombatPawn* OriginalAttacker, ACombatPawn* ParryWinner);
};