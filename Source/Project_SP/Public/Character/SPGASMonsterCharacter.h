#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "Data/CombatEncounterData.h"
#include "SPGASMonsterCharacter.generated.h"



UCLASS()
class PROJECT_SP_API ASPGASMonsterCharacter : public ASPGASCharacterBase
{
    GENERATED_BODY()

public:
    ASPGASMonsterCharacter();
    virtual void PossessedBy(AController* NewController) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatEncounterData> EncounterData;
};