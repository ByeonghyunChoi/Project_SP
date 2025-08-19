#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameAction.generated.h"

class ACombatPawn;
class ABattleManager;
struct FActionData;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UGameAction : public UObject
{
	GENERATED_BODY()
public:
	UGameAction();

	void ExecuteAction(
		ACombatPawn* Instigator,
		const FActionData& ActionData,
		ABattleManager* BattleManagerRef,
		ACombatPawn* TargetPawn,
		const TArray<ACombatPawn*>& TargetPawns);

	bool HasEnoughCost(ACombatPawn* Instigator, const FActionData& ActionData) const;

};