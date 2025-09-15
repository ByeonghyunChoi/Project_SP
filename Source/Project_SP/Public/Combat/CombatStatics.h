#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/StatusEffectData.h" 
#include "CombatStatics.generated.h"

class UAttributesComponent;
class ACombatPawn;
/**
 *
 */
UCLASS()
class PROJECT_SP_API UCombatStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Combat Statics")
	static float CalculateDamage(
		const UAttributesComponent* AttackerStats,
		const UAttributesComponent* TargetStats,
		float SkillCoefficient
	);

	UFUNCTION(BlueprintPure, Category = "Combat Statics")
	static float CalculateStatusEffectDamage(
		const ACombatPawn* Instigator,
		const ACombatPawn* Target,
		const FStatusSubEffect& SubEffect
	);

};