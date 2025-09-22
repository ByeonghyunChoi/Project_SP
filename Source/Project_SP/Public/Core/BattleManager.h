#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CombatTypes.h"
#include "BattleManager.generated.h"

class ACombatPawn;
class UCombatCameraComponent;
enum class EBattleState : uint8;



// 하나의 턴에 대한 정보를 담는 컨텍스트 구조체입니다.
USTRUCT(BlueprintType)
struct FTurnContext
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ACombatPawn> Combatant;

	UPROPERTY()
	ETurnType TurnType;

	FTurnContext(ACombatPawn* InCombatant = nullptr, ETurnType InType = ETurnType::Normal)
		: Combatant(InCombatant), TurnType(InType) {
	}
};


UCLASS()
class PROJECT_SP_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	TArray<FTurnContext> TurnStack;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	EBattleState CurrentBattleState;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle Flow")
	TArray<TObjectPtr<ACombatPawn>> AllCombatants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatCameraComponent> CameraComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndBattle();

	void ProcessTurnFlow(float DeltaTime);
	void PushAndStartTurn(ACombatPawn* Combatant, ETurnType Type);
	void EndCurrentTurn();
	void CheckBattleEndConditions();

	UFUNCTION(BlueprintPure, Category = "Battle Flow")
	ACombatPawn* GetCurrentTurnCharacter() const;

	// PredictOrder 클래스가 사용할 수 있도록 Getter 제공
	const TArray<FTurnContext>& GetTurnStack() const { return TurnStack; }
	const TArray<TObjectPtr<ACombatPawn>>& GetAllCombatants() const { return AllCombatants; }

	FORCEINLINE UCombatCameraComponent* GetCameraComponent() const { return CameraComponent; }

protected:
	// --- 이벤트 핸들러 ---
	UFUNCTION()
	void HandleActionFinished(ACombatPawn* FinishedPawn);

	UFUNCTION()
	void HandleInterruptRequest(ACombatPawn* InInstigator);

	UFUNCTION()
	void HandleCombatantDied(AActor* InInstigator);

private:
	void AdvanceAllActionValues(float DeltaTime);
};