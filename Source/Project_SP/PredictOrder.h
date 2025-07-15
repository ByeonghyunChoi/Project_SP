#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPawn.h"
#include "BattleManager.h"
#include "PredictOrder.generated.h"

USTRUCT(BlueprintType)
struct FSimulatedCombatantData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ACombatPawn> OriginalCombatant;

	UPROPERTY(BlueprintReadWrite)
	float SimulatedActionValue;

	UPROPERTY(BlueprintReadWrite)
	float OriginalSpeed;

	UPROPERTY(BlueprintReadWrite)
	EFaction OriginalFaction;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAlive;

	FSimulatedCombatantData()
		: SimulatedActionValue(0.0f), OriginalSpeed(0.0f), OriginalFaction(EFaction::Player), bIsAlive(false) {
	}

	FSimulatedCombatantData(ACombatPawn* InCombatant)
		: OriginalCombatant(InCombatant), SimulatedActionValue(0.0f), OriginalSpeed(0.0f), OriginalFaction(EFaction::Player), bIsAlive(false)
	{
		if (InCombatant)
		{
			UCharacterStatsComponent* Stats = InCombatant->GetStatsComponent();
			UBattleTurnComponent* TurnComp = InCombatant->FindComponentByClass<UBattleTurnComponent>();

			if (Stats && TurnComp)
			{
				SimulatedActionValue = TurnComp->GetActionValue();
				OriginalSpeed = Stats->GetMovementSpeed();
				OriginalFaction = InCombatant->GetFaction();
				bIsAlive = Stats->GetCurrentHealth() > 0;
			}
		}
	}

	float GetSimulatedTimeLeftToAct() const
	{
		if (OriginalSpeed <= 0.0f) return 99999.0f;
		return FMath::Max(0.0f, (10000.0f - SimulatedActionValue) / OriginalSpeed);
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UPredictOrder : public UActorComponent
{
	GENERATED_BODY()

public:
	UPredictOrder();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "PredictOrder|Events")
	FOnTurnOrderChanged OnTurnOrderChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PredictOrder")
	ABattleManager* BattleManagerRef;

	UFUNCTION(BlueprintCallable, Category = "PredictOrder")
	void RequestTurnOrderUpdate();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PredictOrder")
	TArray<ACombatPawn*> GetPredictedTurnOrder() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsMyOnTurnOrderChangedBound() const;

private:
	struct FCompareSimulatedCombatantData
	{
		bool operator()(const FSimulatedCombatantData& A, const FSimulatedCombatantData& B) const;
	};
};