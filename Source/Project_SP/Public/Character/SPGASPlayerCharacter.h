#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "GameplayTagContainer.h"
#include "SPGASPlayerCharacter.generated.h"

UCLASS()
class PROJECT_SP_API ASPGASPlayerCharacter : public ASPGASCharacterBase
{
	GENERATED_BODY()

public:
	ASPGASPlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> FieldInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TArray<TSubclassOf<class UGameplayAbility>> FieldPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> BattleInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TArray<TSubclassOf<class UGameplayAbility>> BattlePassiveAbilities;

	void GiveAbilities();
};